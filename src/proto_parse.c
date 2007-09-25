//
// Copyright (C) 2002-3 Simon Howard
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

// parsing of received packets
// this code runs on the assumption that the packets being given to
// it are well formed and valid; it does not do any checking on the data
// received.
//
// TODO: proto_verify.c for verifification of packets before they are
// parsed.

#include "sysheaders.h"

#include "error.h"
#include "packet.h"
#include "protocol.h"
#include "sendatom.h"

// alpha value used for estimating round trip time

#define RTT_ALPHA 0.9

int irmo_proto_use_preexec = 1;

// only the low 16 bits of the stream position is sent
// therefore we must expand positions we get based on the
// current position

static int get_stream_position(int current, int low)
{
	int newpos = (current & ~0xffff) | low;

	// if new position is greater than 32768 behind the
	// current position, we have probably wrapped around
	// similarly wrap around the other way
	
	if (current - newpos > 0x8000)
		newpos += 0x10000;
	else if (newpos - current > 0x8000)
		newpos -= 0x10000;

	return newpos;
}


static void proto_parse_insert_atom(IrmoClient *client,
				    IrmoSendAtom *atom,
				    int seq)
{
	int index = seq - client->recvwindow_start;
	int i;

	// increase the receive window size if neccesary
	
	if (index >= client->recvwindow_size) {
		int newsize = index+1;

		client->recvwindow
			= irmo_renew(IrmoSendAtom *,
				     client->recvwindow,
				     newsize);

		for (i=client->recvwindow_size; i<newsize; ++i)
			client->recvwindow[i] = NULL;

		client->recvwindow_size = newsize;
	}

	// delete old sendatoms in the same position (assume
	// new retransmitted atoms are more up to date)
	
	if (client->recvwindow[index])
		irmo_sendatom_free(client->recvwindow[index]);

	client->recvwindow[index] = atom;
}

static void proto_parse_packet_cluster(IrmoClient *client, IrmoPacket *packet)
{
	unsigned int i;
	int seq;
	int start;
	
	// get the start position
	
	irmo_packet_readi16(packet, &i);

	start = get_stream_position(client->recvwindow_start, i);

	//printf("stream position: %i->%i\n", i, start);
	
	for (seq=start;;) {
		IrmoSendAtomClass *klass;
		IrmoSendAtomType atomtype;
		unsigned int natoms;
		unsigned int byte;
		
		// read type/count byte
		// if none, end of packet
		
		if (!irmo_packet_readi8(packet, &byte))
			break;

		atomtype = (byte >> 5) & 0x07;
		natoms = (byte & 0x1f) + 1;

		klass = irmo_sendatom_types[atomtype];

		//printf("%i atoms, type %i\n", natoms, atomtype);
		
		for (i=0; i<natoms; ++i, ++seq) {
			IrmoSendAtom *atom;

			atom = klass->read(packet);
			atom->client = client;
			atom->seqnum = seq;

			// set the need_ack flag if this is at the start
			// or before the start of the recvwindow. We only
			// send an ack when the atom is at the start of
			// the recvwindow (seq == client->recvwindow_start)
			// as this allows us to advance the window
			// or if it is too old (as this could be resend
			// because our previous ack was lost

			if (seq <= client->recvwindow_start)
				client->need_ack = 1;
			
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

	if (irmo_proto_use_preexec)
		irmo_client_run_preexec(client,
					start - client->recvwindow_start,
					seq - client->recvwindow_start);
}

static void proto_parse_ack(IrmoClient *client, int ack)
{
	int seq;
	int relative;
	int i;
	
	//printf("got an ack: %i\n", ack);
	
	// extrapolate the high bits from the low 16 bits

	seq = get_stream_position(client->sendwindow_start, ack);

	// get position in sendwindow array, relative to the start

	relative = seq - client->sendwindow_start;

	if (relative <= 0) {
		// already acked this far

		return;
	}

	if (relative > client->sendwindow_size) {
		// bogus ack
		// we havent even sent this far in the stream yet

		irmo_error_report("proto_parse_ack",
				  "bogus ack (%i < %i < %i)",
				  client->sendwindow_start, seq,
				  client->sendwindow_start + client->sendwindow_size);
		return;
	}

	// we got a valid ack. open up the send window a bit more
	// if we are above the slow start congestion threshold, open
	// slowly
	
	if (client->cwnd < client->ssthresh)
		client->cwnd += PACKET_THRESHOLD;
	else
		client->cwnd +=
			(PACKET_THRESHOLD * PACKET_THRESHOLD) / client->cwnd;
	
	// We are acking something valid and advancing the window
	// Assume this is because the first atom in the window has
	// been cleared. Therefore we can estimate the round-trip time
	// by subtracting the current time from the send time of the
	// first atom in the send window.
	// If the atom was resent, this cannot be used.

	if (!client->sendwindow[0]->resent) {
		unsigned int nowtime, rtt;
		int deviation;
		
                nowtime = irmo_get_time();

                rtt = nowtime - client->sendwindow[0]->sendtime;

		deviation = abs(rtt - client->rtt);
		
		client->rtt = RTT_ALPHA * client->rtt
			+ (1-RTT_ALPHA) * rtt;

		client->rtt_deviation = RTT_ALPHA * client->rtt_deviation
			+ (1-RTT_ALPHA) * deviation;

		// reset backoff back to 1 now we have a valid packet

		client->backoff = 1;
	}	
	
	// need to move the sendwindow along
	// destroy the atoms in the area acked
	
	for (i=0; i<relative; ++i)
		irmo_sendatom_free(client->sendwindow[i]);

	memcpy(client->sendwindow,
	       client->sendwindow + relative,
	       sizeof(*client->sendwindow)
	         * (client->sendwindow_size - relative));

	client->sendwindow_start += relative;
	client->sendwindow_size -= relative;
}

void irmo_proto_parse_packet(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;

	// verify packet before parsing for security
	
	if (!irmo_proto_verify_packet(packet)) {
		irmo_error_report("proto_parse_packet",
				  "dropped packet (failed security verification)");
		return;
	}

	// read ack field if there is one

	if (packet->flags & PACKET_FLAG_ACK) {
		unsigned int i;

		irmo_packet_readi16(packet, &i);

		proto_parse_ack(client, i);
	}

	if (packet->flags & PACKET_FLAG_DTA) {
		proto_parse_packet_cluster(client, packet);

		irmo_client_run_recvwindow(client);
	}
}

