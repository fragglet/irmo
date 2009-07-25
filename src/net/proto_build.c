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

#include "arch/sysheaders.h"

#include <irmo/packet.h>

#include "client.h"
#include "client_sendq.h"
#include "protocol.h"
#include "sendatom.h"

// Get maximum send window size for the specified client

static unsigned int client_sendwindow_max(IrmoClient *client)
{
        unsigned int limit;

	// The limit is the minimum of the local and remote limits,
        // if either of them are set.

        limit = client->local_sendwindow_max;

        if (client->remote_sendwindow_max != 0) {
                if (limit == 0 || client->remote_sendwindow_max < limit) {
                        limit = client->remote_sendwindow_max;
                }
        }

        // If no limit has been set at all, try to automatically determine
        // the limit.

        if (limit != 0) {
                return limit;
        } else {
                return (unsigned int) client->cwnd;
        }
}

// Data gets pumped into the sendwindow until it reaches the send
// window size, then no more is added.

static void client_pump(IrmoClient *client)
{
        IrmoSendAtom *atom;
	unsigned int current_size = 0;
	unsigned int i;
	unsigned int sendwindow_max;
	
	// if queue is already empty, this is a nonissue
	
	if (irmo_queue_is_empty(client->sendq)) {
		return;
        }

        // Calculate maximum sendwindow size

        sendwindow_max = client_sendwindow_max(client);

        // Get the current sendwindow size.

	for (i=0; i<client->sendwindow_size; ++i) {
		current_size += client->sendwindow[i]->len;
        }

	// adding things in until we run out of space or atoms to add
	
	while (current_size < sendwindow_max
	    && !irmo_queue_is_empty(client->sendq)
	    && client->sendwindow_size < MAX_SENDWINDOW) {

		// pop another from the sendq and add to the sendwindow

		atom = irmo_client_sendq_pop(client);		
		atom->sendtime = IRMO_ATOM_UNSENT;
		
		client->sendwindow[client->sendwindow_size] = atom;
		atom->seqnum = client->sendwindow_start
			     + client->sendwindow_size;
		
		++client->sendwindow_size;
		
		// keep track of size
		
		current_size += atom->len;
	}
}

// Update the "resent" flag for the atoms in the specified range.

static void client_flag_resends(IrmoClient *client,
                                unsigned int start, unsigned int end)
{
        unsigned int i;

	// Set the resent flag on all atoms that were previously sent.
        
        for (i=start; i <= end; ++i) {
                if (client->sendwindow[i]->sendtime != IRMO_ATOM_UNSENT) {
                        client->sendwindow[i]->resent = 1;
                }
        }

	// If we are resending the first atom, use exponential backoff and
        // double the resend time every time we resend
	
	if (start == 0 && client->sendwindow[0]->resent) {

		// If this is the first time we have missed a packet, it's
		// possible we're experiencing congestion.
		// Reset the send window size back to one packet and save the 
		// slow-start threshold.
		
		if (client->backoff == 1) {
			client->ssthresh = (unsigned int) (client->cwnd / 2);
			client->cwnd = PACKET_THRESHOLD;
		}
		
		client->backoff *= 2;
		//printf("backoff now %i\n", client->backoff);
	}
}

// Expand a list of atoms to send

static void client_expand_packet(IrmoClient *client, 
                                 unsigned int *start, unsigned int *end)
{
        unsigned int backstart;

	// move the start back to cover all null atoms that prefix this
	// data segment. in running games we typically repeatedly change 
	// the same objects on a clock. each clock, the previous changes
	// are then nullified. this means in sending new data we typically
	// have lots of NULL atoms before the start of our new data.
	// as NULL atoms compress very well and take up very little room,
	// include these as it may reduce the need for retransmissions.
	
        backstart = *start;

        while (backstart > 0 
            && client->sendwindow[backstart-1] != NULL
            && client->sendwindow[backstart-1]->klass == &irmo_null_atom) {
                --backstart;
        }

        *start = backstart;
}

// Set the send time for atoms in the specified range.

static void client_set_send_times(IrmoClient *client,
                                  unsigned int start, unsigned int end,
                                  unsigned int nowtime)
{
        unsigned int i;

        // Set the send time for all atoms.
        
        for (i=start; i <= end; ++i) {
                client->sendwindow[i]->sendtime = nowtime;
        }
}

// Build a packet to send to the specified client containing the
// atoms from the sendwindow in the range start...end

static IrmoPacket *client_build_packet(IrmoClient *client,
                                       unsigned int start, unsigned int end)
{
	IrmoPacket *packet;
	unsigned int i, n;

	// Make a new packet
	
	packet = irmo_packet_new();

	// Packet header.
	
	irmo_packet_writei16(packet, PACKET_FLAG_ACK|PACKET_FLAG_DTA);
	
	// In sending stream positions we only send the low 16
	// bits. the higher bits can be implied by their current
	// position.
	
        // Add the last acked point in the stream.

	irmo_packet_writei16(packet, client->recvwindow_start & 0xffff);
	client->need_ack = 0;

	// Start position in stream
	
	irmo_packet_writei16(packet, 
			     (client->sendwindow_start + start) & 0xffff);

	//printf("-- build_packet: range %i-%i\n", start,end);

	// Add all sendatoms in the range specified

        i = start;

	while (i <= end) {
		IrmoSendAtomClass *klass;

		klass = client->sendwindow[i]->klass;

		// Group up to 32 sendatoms of the same type together
		// we send the number of EXTRA sendatoms after
		// the starting one: the first one is implied. after that
		// we can specify up to 31 of the same type that follow
		
		for (n=1; i+n<=end && n<32; ++n) {
			if (klass != client->sendwindow[i+n]->klass) {
				break;
                        }
                }

		//printf("-- build_packet: group length %i, type %i\n",
		//n, client->sendwindow[i]->type);

		// store extra count in the low bits, type in the high bits

		irmo_packet_writei8(packet, (klass->type << 5) | (n-1));

		// add atoms

		for (; n>0; --n, ++i) {
			klass->write(client->sendwindow[i], packet);
		}
	}

	return packet;
}

// Determine if a sendatom has timed out and needs to be resent.

static int atom_timed_out(IrmoSendAtom *atom, unsigned int nowtime,
                          unsigned int timeout_length)
{
        if (atom->sendtime == IRMO_ATOM_UNSENT) {
                return 0;
        }

        return nowtime >= atom->sendtime + timeout_length;
}

// Check if a given atom needs to be [re]sent.

static int atom_needs_send(IrmoSendAtom *atom, unsigned int nowtime,
                           unsigned int timeout_length)
{
        // Not sent yet? It needs to be sent now.

        if (atom->sendtime == IRMO_ATOM_UNSENT) {
                return 1;
        }

        // Timed out?  Send it again.

        return atom_timed_out(atom, nowtime, timeout_length);
}

// Check a client's send queue for packets that need to be [re]transmitted.

static void client_send_data(IrmoClient *client,
                             unsigned int timeout_length)
{
        IrmoPacket *packet;
        unsigned int len;
        unsigned int start, end;
        unsigned int i;
        unsigned int nowtime;

        // All atoms where nowtime >= atom->sendtime + timeout_length
        // need to be resent.
     
        nowtime = irmo_get_time();

        i = 0;

        while (i < client->sendwindow_size) {

		// Search forward until we find the start of a block
                // of atoms to send.

                for (; i<client->sendwindow_size; ++i) {

                        if (atom_needs_send(client->sendwindow[i],
                                            nowtime, timeout_length)) {

                                // This is the first of a block of atoms
                                // to send.

                                break;
                        }
		}

                // Reached the end of the send window size?

		if (i >= client->sendwindow_size) {
			break;
                }

		start = i;

                // Find the end point of a packet.  Keep searching forward
                // in the send window until an atom is found which does
                // not need to be sent.  Also, count the number of bytes
                // of data, so we do not exceed the maximum packet size.
		
		len = 0;
		
                for (; i < client->sendwindow_size; ++i) {

                        if (!atom_needs_send(client->sendwindow[i],
                                             nowtime, timeout_length)) {

                                // This packet does not need to be sent.
                                // This is the end of our run of packets to
                                // send.

                                break;
                        }

                        if (len >= PACKET_THRESHOLD) {

                                // We have exceeded the maximum size of 
                                // a packet.  No more atoms can be added
                                // to this packet.
                            
                                break;
                        }
                        
                        // add this atom

			//printf("atom %i out of date\n", i);
			len += client->sendwindow[i]->len;
		}

                end = i - 1;

		//printf("sendpacket: %i->%i\n", start, end);
		
                // Update the "resent" flags for the atoms to be sent.
                // This is deliberately done before null expansion
                // (below), as null atoms are deliberately sent multiple
                // times; it is not a retransmission.

                client_flag_resends(client, start, end);

                // Expand packet list to include neighbouring null atoms.

                client_expand_packet(client, &start, &end);

                // Update the send times for the atoms to be transmitted.

                client_set_send_times(client, start, end, nowtime);

                // Build and transmit a packet containing the atoms.

		packet = client_build_packet(client, start, end);

		irmo_net_socket_send_packet(client->server->socket,
				            client->address,
				            packet);

		irmo_packet_free(packet);
	}
}

static void client_send_ack(IrmoClient *client)
{
        IrmoPacket *packet;

        packet = irmo_packet_new();

        // only ack flag is sent, not dta as there is no data
        
        irmo_packet_writei16(packet, PACKET_FLAG_ACK);
        irmo_packet_writei16(packet, client->recvwindow_start & 0xffff);

        // send packet

        irmo_net_socket_send_packet(client->server->socket,
                                    client->address,
                                    packet);

        irmo_packet_free(packet);
}

// maximum timeout length (ms)
// beyond this length, the backoff has got too high and the client is
// disconnected

#define MAX_TIMEOUT 40000

void irmo_proto_run_client(IrmoClient *client)
{
	unsigned int timeout_length;

	// If we have been trying to resend for too long, give up and
	// time out.

	timeout_length = irmo_client_timeout_time(client) * client->backoff;

	if (timeout_length > MAX_TIMEOUT) {

                // Disconnect the dead client.

		client->state = CLIENT_DISCONNECTED;
		irmo_client_callback_raise(&client->disconnect_callbacks,
					   client);
	}
	
	// Pump new atoms to send window.
	
	client_pump(client);

        // Check for data to send

        client_send_data(client, timeout_length);

	// Possibly we need to send an ack for something we have received.
	// If we have nothing in our send window to send, we still need
	// to send an acknowledgement back.

	if (client->need_ack) {
                client_send_ack(client);
                client->need_ack = 0;
	}
}

