//
// Copyright (C) 2002-2008 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

// Parsing of received packets.
// This code runs on the assumption that the packets being given to
// it are well formed and valid; it does not do any checking on the data
// received.
//

#include "arch/sysheaders.h"
#include "base/alloc.h"
#include "base/error.h"

#include <irmo/packet.h>

#include "protocol.h"
#include "sendatom.h"

// alpha value used for estimating round trip time

#define RTT_ALPHA 0.9f

int irmo_proto_use_preexec = 1;

// only the low 16 bits of the stream position is sent
// therefore we must expand positions we get based on the
// current position

static unsigned int get_stream_position(unsigned int current, unsigned int low)
{
	unsigned int newpos = (current & ~0xffff) | low;

	// if new position is greater than 32768 behind the
	// current position, we have probably wrapped around
	// similarly wrap around the other way
	
	if (current > newpos && current - newpos > 0x8000) {
		newpos += 0x10000;
	} else if (newpos > current && newpos - current > 0x8000) {
		newpos -= 0x10000;
        }

	return newpos;
}

static void proto_parse_insert_atom(IrmoClient *client,
				    IrmoSendAtom *atom,
				    unsigned int seq)
{
	unsigned int index = seq - client->recvwindow_start;
	unsigned int i;

	// increase the receive window size if neccesary
	
	if (index >= client->recvwindow_size) {
		unsigned int newsize = index + 1;

		client->recvwindow = irmo_renew(IrmoSendAtom *,
                                                client->recvwindow,
                                                newsize);

		for (i=client->recvwindow_size; i<newsize; ++i) {
			client->recvwindow[i] = NULL;
                }

		client->recvwindow_size = newsize;
	}

	// delete old sendatoms in the same position (assume
	// new retransmitted atoms are more up to date)
	
	if (client->recvwindow[index] != NULL) {
		irmo_sendatom_free(client->recvwindow[index]);
        }

	client->recvwindow[index] = atom;
}

static void proto_parse_packet_data(IrmoClient *client, IrmoPacket *packet)
{
	unsigned int i;
	unsigned int seq;
	unsigned int start;

	// get the start position

	irmo_packet_readi16(packet, &i);

	start = get_stream_position(client->recvwindow_start, i);

	//printf("stream position: %i->%i\n", i, start);

        seq = start;

	for (;;) {
		IrmoSendAtomClass *klass;
		IrmoSendAtomType atomtype;
		unsigned int natoms;
		unsigned int byte;

		// read type/count byte
		// if none, end of packet
		
		if (!irmo_packet_readi8(packet, &byte)) {
			break;
                }

		atomtype = (byte >> 5) & 0x07;
		natoms = (byte & 0x1f) + 1;

		klass = irmo_sendatom_types[atomtype];

		//printf("%i atoms, type %i\n", natoms, atomtype);
		
		for (i=0; i<natoms; ++i, ++seq) {
			IrmoSendAtom *atom;

			atom = klass->read(packet, client);
			atom->client = client;
			atom->seqnum = seq;

			// set the need_ack flag if this is at the start
			// or before the start of the recvwindow. We only
			// send an ack when the atom is at the start of
			// the recvwindow (seq == client->recvwindow_start)
			// as this allows us to advance the window
			// or if it is too old (as this could be resend
			// because our previous ack was lost

			if (seq <= client->recvwindow_start) {
				client->need_ack = 1;
                        }
			
			// too old?
			
			if (seq < client->recvwindow_start) {
				irmo_sendatom_free(atom);
				continue;
			}

			// insert atom into receive window

			proto_parse_insert_atom(client, atom, seq);
		}
	}

	// try to preexec the new data

	if (irmo_proto_use_preexec) {
		irmo_client_run_preexec(client, start, seq);
        }
}

// Perform a low pass filter of RTT values

static float rtt_low_pass_filter(float last_value,
                                 unsigned int new_value)
{
        float result;

        result = ( last_value * RTT_ALPHA )
               + ( (float) new_value * (1 - RTT_ALPHA) );

        return result;
}

// Update the congestion control values for the given client, after 
// receiving a successful acknowledgement.

static void proto_update_cc_values(IrmoClient *client)
{
        unsigned int nowtime, rtt;
        unsigned int deviation;

	// We got a valid ack. open up the send window a bit more.
	// If we are above the slow start congestion threshold, open
	// slower.

	if (client->cwnd < client->ssthresh) {
		client->cwnd += PACKET_THRESHOLD;
	} else {
		client->cwnd +=
			(PACKET_THRESHOLD * PACKET_THRESHOLD) / client->cwnd;
        }

        //
	// We are acking something valid and advancing the window
	// Assume this is because the first atom in the window has
	// been cleared. Therefore we can estimate the round-trip time
	// by subtracting the current time from the send time of the
	// first atom in the send window.
        //
	// If the atom was resent, this cannot be used.
        //

	if (!client->sendwindow[0]->resent) {
                nowtime = irmo_get_time();

                // Round-trip time

                rtt = nowtime - client->sendwindow[0]->sendtime;

		deviation = (unsigned int) abs((int) rtt - (int) client->rtt);

                client->rtt
                        = rtt_low_pass_filter(client->rtt, rtt);
                client->rtt_deviation
                        = rtt_low_pass_filter(client->rtt_deviation,
                                              deviation);

		// Reset exponential backoff now that we have a valid packet

		client->backoff = 1;
	}
}

static void advance_send_window(IrmoClient *client, unsigned int length)
{
        IrmoSendAtom *atom;
        unsigned int i;

	// destroy all atoms in the area acked

	for (i=0; i<length; ++i) {

                atom = client->sendwindow[i];

                // Signal the atom that it has been acknowledged.

                if (atom->klass->acked != NULL) {
                        atom->klass->acked(atom);
                }

		irmo_sendatom_free(atom);
        }

        // advance the send window forward

	memcpy(client->sendwindow,
	       client->sendwindow + length,
	       sizeof(*client->sendwindow)
	         * (client->sendwindow_size - length));

        // update counters

	client->sendwindow_start += length;
	client->sendwindow_size -= length;
}

static void proto_parse_ack(IrmoClient *client, unsigned int ack)
{
	unsigned int seq;
	unsigned int relative;

	//printf("got an ack: %i\n", ack);

	// extrapolate the high bits from the low 16 bits

	seq = get_stream_position(client->sendwindow_start, ack);

	// get position in sendwindow array, relative to the start

        if (seq <= client->sendwindow_start) {
                return;
        }

	relative = seq - client->sendwindow_start;

        // Check that the acknowledgement is within the range of the 
        // send window.

	if ((unsigned) relative > client->sendwindow_size) {
/*
                irmo_warning_message("proto_parse_ack",
                        "bogus ack (%i < %i < %i)",
                        client->sendwindow_start, seq,
                        client->sendwindow_start + client->sendwindow_size);
*/
		return;
	}

        // Update congestion control values

        proto_update_cc_values(client);
        advance_send_window(client, relative);
}

void irmo_proto_parse_packet(IrmoPacket *packet,
                             IrmoClient *client,
                             unsigned int flags)
{
	// verify packet before parsing for security
	
	if (!irmo_proto_verify_packet(packet, client, flags)) {
/*
		irmo_warning_message("irmo_proto_parse_packet",
                      "dropped packet (failed security verification)");
*/
		return;
	}

	// read ack field if there is one

	if ((flags & PACKET_FLAG_ACK) != 0) {
		unsigned int i;

		irmo_packet_readi16(packet, &i);

		proto_parse_ack(client, i);
	}

	if ((flags & PACKET_FLAG_DTA) != 0) {
		proto_parse_packet_data(client, packet);

		irmo_client_run_recvwindow(client);
	}
}

