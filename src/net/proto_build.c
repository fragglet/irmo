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

#include "arch/sysheaders.h"
#include "base/util.h"

#include "netbase/netlib.h"
#include "netbase/packet.h"

#include "protocol.h"
#include "sendatom.h"

static void proto_atom_resent(IrmoClient *client, int i)
{
	// set resent flag
	
	client->sendwindow[i]->resent = 1;
	
	// if we are resending the first atom,
	// use exponential backoff and double the
	// resend time every time we resend
	
	if (i == 0) {

		// if this is the first time we have missed a packet, it's
		// possible we're experiencing congestion.
		// reset the send window size back to one packet and save the 
		// slow-start threshold
		
		if (client->backoff == 1) {
			client->ssthresh = client->cwnd / 2;
			client->cwnd = PACKET_THRESHOLD;
		}
		
		client->backoff *= 2;
		//printf("backoff now %i\n", client->backoff);
	}
	
}

static IrmoPacket *proto_build_packet(IrmoClient *client, int start, int end)
{
	IrmoPacket *packet;
	int i, n;
	int backstart;
        unsigned int nowtime;

        nowtime = irmo_get_time();

	// make a new packet
	
	packet = irmo_packet_new();

	// header
	// always send last-acked point 
	
	irmo_packet_writei16(packet, PACKET_FLAG_ACK|PACKET_FLAG_DTA);
	
	// in sending stream positions we only send the low 16
	// bits. the higher bits can be implied by their current
	// position
	
	// last acked point
	
	irmo_packet_writei16(packet, client->recvwindow_start & 0xffff);
	client->need_ack = 0;

	// move the start back to cover all null atoms that prefix this
	// data segment. in running games we typically repeatedly change 
	// the same objects on a clock. each clock, the previous changes
	// are then nullified. this means in sending new data we typically
	// have lots of NULL atoms before the start of our new data.
	// as NULL atoms compress very well and take up very little room,
	// include these as it may reduce the need for retransmissions.
	
	for (backstart=start; 
	     backstart > 0 
		&& client->sendwindow[backstart-1]
		&& client->sendwindow[backstart-1]->klass==&irmo_null_atom;
	     --backstart);
	
	// start position in stream
	
	irmo_packet_writei16(packet, 
			     (client->sendwindow_start + backstart) & 0xffff);

	//printf("-- build_packet: range %i-%i\n", start,end);
	// add all sendatoms in the range specified

	for (i=backstart; i<=end;) {
		IrmoSendAtomClass *klass;

		klass = client->sendwindow[i]->klass;

		// group up to 32 sendatoms of the same type together
		// we send the number of EXTRA sendatoms after
		// the starting one: the first one is implied. after that
		// we can specify up to 31 of the same type that follow
		
		for (n=1; i+n<=end && n<32; ++n)
			if (klass != client->sendwindow[i+n]->klass)
				break;

		//printf("-- build_packet: group length %i, type %i\n",
		//n, client->sendwindow[i]->type);
		       

		// store extra count in the low bits, type in the high bits

		irmo_packet_writei8(packet, (klass->type << 5) | (n-1));

		// add atoms

		for (; n>0; --n, ++i) {
			klass->write(client->sendwindow[i], packet);

			// if this is a prefixed NULL atom, ignore the
			// resend code. we are resending deliberately

			if (client->sendwindow[i]->sendtime != IRMO_ATOM_UNSENT
			 && i >= start)
				proto_atom_resent(client, i);
			
			// store send time in atoms

			client->sendwindow[i]->sendtime = nowtime;
		}
	}

	return packet;
}

// data gets pumped into the sendwindow until it passes this threshold
// size, then no more is added

// this is currently static. TODO: modify based on network conditions
// (eg. bigger for high bandwidth connections, smaller for low bandwidth)

#define SENDWINDOW_THRESHOLD 4096

static void proto_pump_client(IrmoClient *client)
{
	int current_size = 0;
	int i;
	int sendwindow_max;
	
	// if queue is already empty, this is a nonissue
	
	if (irmo_queue_is_empty(client->sendq))
		return;

	for (i=0; i<client->sendwindow_size; ++i)
		current_size += client->sendwindow[i]->len;

	// determine the maximum send window size
	// if no maximum size has been provided, try to determine
	// one automatically from congestion avoidance algorithms.
	// otherwise, use the minimum of those provided to the
	// server and client

	if (client->local_sendwindow_max) {
		if (client->remote_sendwindow_max
		 && client->remote_sendwindow_max
		         < client->local_sendwindow_max)
			sendwindow_max = client->remote_sendwindow_max;
		else
			sendwindow_max = client->local_sendwindow_max;
	} else {
		if (client->remote_sendwindow_max)
			sendwindow_max = client->remote_sendwindow_max;
		else
			sendwindow_max = client->cwnd;
	}

	// adding things in until we run out of space or atoms to add
	
	while (current_size < sendwindow_max
	       && !irmo_queue_is_empty(client->sendq)
	       && client->sendwindow_size < MAX_SENDWINDOW) {
		IrmoSendAtom *atom;

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

// maximum timeout length (ms)
// beyond this length, the backoff has got too high and the client is
// disconnected

#define MAX_TIMEOUT 40000

void irmo_proto_run_client(IrmoClient *client)
{
        unsigned int nowtime;
	int timeout_length;
	int i;

	// If we have been trying to resend for too long, give up and
	// time out.

	timeout_length = irmo_client_timeout_time(client) * client->backoff;

	if (timeout_length > MAX_TIMEOUT) {

                // Disconnect the dead client.

		client->state = CLIENT_DISCONNECTED;
		irmo_client_callback_raise(client->disconnect_callbacks,
					   client);
	}
	
	// Pump new atoms to send window.
	
	proto_pump_client(client);

        // All atoms where nowtime >= atom->sendtime + timeout_length
        // need to be resent.
     
        nowtime = irmo_get_time();

	//printf("timeout for client: %i ms (%i, %i)\n",
	//timeout_length,
	//(int) client->rtt,
	//(int) client->rtt_deviation);
	
	//for (i=0; i<client->sendwindow_size; ++i) {
	//printf("%i: %i\n", i, client->sendwindow[i]->sendtime);
	//}

	for (i=0; i<client->sendwindow_size; ) {
		IrmoPacket *packet;
		int len;
		int start;

		//printf("find start\n");
		
		// search forward until we find the start of a block

		while (i<client->sendwindow_size) {

                        if (client->sendwindow[i]->sendtime == IRMO_ATOM_UNSENT
                         || nowtime >= client->sendwindow[i]->sendtime + timeout_length) {

                                // This atom has either not yet been
                                // sent, or has timed out.  Therefore, this
                                // atom needs to be sent, and marks the start
                                // of a block of atoms to be sent.
                                break;
                        }

			//printf("atom %i not expired yet\n", i);
			++i;
		}

                // Reached the end of the send window size?

		if (i >= client->sendwindow_size)
			break;

                // Find the end point of a packet.  Keep searching forward
                // in the send window until an atom is found which does
                // not need to be sent.  Also, count the number of bytes
                // of data, so we do not exceed the maximum packet size.
		
		start = i;
		len = 0;
		
		while (i<client->sendwindow_size) {

                        if (nowtime < client->sendwindow[i]->sendtime + timeout_length
                         && client->sendwindow[i]->sendtime != IRMO_ATOM_UNSENT) {
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
			++i;
		}

		//printf("packet %i->%i\n", start, i-1);

		// build a packet 

		packet = proto_build_packet(client, start, i-1);

		//printf("sendpacket: %i->%i\n", start, i-1);
		
                // Transmit the packet.

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		//printf("free packet\n");
		
		// finished using packet
		
		irmo_packet_free(packet);
	}

	//printf("finished\n");

	// Possibly we need to send an ack for something we have received.
	// If we have nothing in our send window to send, we still need
	// to send an acknowledgement back.

	if (client->need_ack) {
		IrmoPacket *packet;

		//printf("send ack to client\n");

		packet = irmo_packet_new();

		// only ack flag is sent, not dta as there is no data
		
		irmo_packet_writei16(packet, PACKET_FLAG_ACK);
		irmo_packet_writei16(packet, client->recvwindow_start & 0xffff);
		client->need_ack = 0;

		// send packet

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		irmo_packet_free(packet);
	}
}

