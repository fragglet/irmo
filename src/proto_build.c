// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "packet.h"
#include "protocol.h"
#include "sendatom.h"

static void proto_add_change_atom(IrmoPacket *packet, IrmoSendAtom *atom)
{
	IrmoObject *obj = atom->data.change.object;
	int bitmap_size;
	int i, j;

	// include the object class number
	// this is neccesary otherwise the packet can be ambiguous to
	// decode (if we receive a change atom for an object which has
	// not yet been received, for example)

	packet_writei8(packet, obj->objclass->index);
	
	// send object id
	
	packet_writei16(packet, obj->id);

	// build and send bitmap

	bitmap_size = (obj->objclass->nvariables + 7) / 8;

	for (i=0; i<bitmap_size; ++i) {
		guint8 b;

		// build a byte at a time

		b = 0;
		
		for (j=0; j<8 && i*8+j<obj->objclass->nvariables; ++j) {
			if (atom->data.change.changed[i*8 + j]) {
				b |= 1 << j;
			}
		}

		packet_writei8(packet, b);
	}

	// send variables

	for (i=0; i<obj->objclass->nvariables; ++i) {

		// check we are sending this variable

		if (!atom->data.change.changed[i])
			continue;

		// todo

		switch (obj->objclass->variables[i]->type) {
		case IRMO_TYPE_INT8:
			packet_writei8(packet, obj->variables[i].i8);
			break;
		case IRMO_TYPE_INT16:
			packet_writei16(packet, obj->variables[i].i16);
			break;
		case IRMO_TYPE_INT32:
			packet_writei32(packet, obj->variables[i].i32);
			break;
		case IRMO_TYPE_STRING:
			packet_writestring(packet, obj->variables[i].s);
			break;
		}
	}
}

static void proto_add_method_atom(IrmoPacket *packet, IrmoSendAtom *atom)
{
	MethodSpec *method = atom->data.method.spec;
	IrmoVariable *args = atom->data.method.args;
	int i;
	
	// send method index
	
	packet_writei8(packet, method->index);

	// send arguments

	for (i=0; i<method->narguments; ++i) {
		switch (method->arguments[i]->type) {
		case IRMO_TYPE_INT8:
			packet_writei8(packet, args[i].i8);
			break;
		case IRMO_TYPE_INT16:
			packet_writei16(packet, args[i].i16);
			break;
		case IRMO_TYPE_INT32:
			packet_writei16(packet, args[i].i32);
			break;
		case IRMO_TYPE_STRING:
			packet_writestring(packet, args[i].s);
			break;
		}
	}
}

static void proto_add_atom(IrmoPacket *packet, IrmoSendAtom *atom)
{
	switch (atom->type) {
	case ATOM_NEW:
		packet_writei16(packet, atom->data.newobj.id);
		packet_writei8(packet, atom->data.newobj.classnum);
		break;
	case ATOM_CHANGE:
		proto_add_change_atom(packet, atom);
		break;
	case ATOM_DESTROY:
		packet_writei16(packet, atom->data.destroy.id);
		break;
	case ATOM_METHOD:
		proto_add_method_atom(packet, atom);
		break;
	case ATOM_NULL:
		break;
	case ATOM_SENDWINDOW:
		packet_writei16(packet, atom->data.sendwindow.max);
		break;
	}
}

G_INLINE_FUNC void proto_atom_resent(IrmoClient *client, int i)
{
	// set resent flag
	
	client->sendwindow[i]->resent = TRUE;
	
	// if we are resending the first atom,
	// use exponential backoff and double the
	// resend time every time we resend
	
	if (i == 0) {

		// if this is the first time we have missed a packet, its
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

IrmoPacket *proto_build_packet(IrmoClient *client, int start, int end)
{
	struct timeval nowtime;
	IrmoPacket *packet;
	int i, n;
	int backstart;

	gettimeofday(&nowtime, NULL);
	
	// make a new packet
	
	packet = packet_new(5);

	// header
	// always send last-acked point 
	
	packet_writei16(packet, PACKET_FLAG_ACK|PACKET_FLAG_DTA);
	
	// in sending stream positions we only send the low 16
	// bits. the higher bits can be implied by their current
	// position
	
	// last acked point
	
	packet_writei16(packet, client->recvwindow_start & 0xffff);
	client->need_ack = FALSE;

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
		&& client->sendwindow[backstart-1]->type==ATOM_NULL;
	     --backstart);
	
	// start position in stream
	
	packet_writei16(packet, 
			(client->sendwindow_start + backstart) & 0xffff);

	//printf("-- build_packet: range %i-%i\n", start,end);
	// add all sendatoms in the range specified

	for (i=backstart; i<=end;) {

		// group up to 32 sendatoms of the same type together
		// we send the number of EXTRA sendatoms after
		// the starting one: the first one is implied. after that
		// we can specify up to 31 of the same type that follow
		
		for (n=1; i+n<=end && n<32; ++n)
			if (client->sendwindow[i+n]->type
			 != client->sendwindow[i]->type)
				break;

		//printf("-- build_packet: group length %i, type %i\n",
		//n, client->sendwindow[i]->type);
		       

		// store extra count in the low bits, type in the high bits

		packet_writei8(packet,
			       (client->sendwindow[i]->type << 5) | (n-1));

		// add atoms

		for (; n>0; --n, ++i) {
			proto_add_atom(packet, client->sendwindow[i]);

			// if this is a prefixed NULL atom, ignore the
			// resend code. we are resending deliberately

			if (client->sendwindow[i]->sendtime.tv_sec
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
	
	if (g_queue_is_empty(client->sendq))
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
	       && !g_queue_is_empty(client->sendq)
	       && client->sendwindow_size < MAX_SENDWINDOW) {
		IrmoSendAtom *atom;

		// pop another from the sendq and add to the sendwindow

		atom = irmo_client_sendq_pop(client);		
		atom->sendtime.tv_sec = 0;
		
		client->sendwindow[client->sendwindow_size++] = atom;

		// keep track of size
		
		current_size += atom->len;
	}
}

// maximum timeout length (ms)
// beyond this length, the backoff has got too high and the client is
// disconnected

#define MAX_TIMEOUT 40000

void proto_run_client(IrmoClient *client)
{
	struct timeval nowtime, timeout_time, timeout_length;
	int timeout_length_ms;
	int i;

	// if we have been trying to resend for too long, give up and
	// time out

	timeout_length_ms = irmo_client_timeout_time(client) * client->backoff;

	if (timeout_length_ms > MAX_TIMEOUT) {
		client->state = CLIENT_DISCONNECTED;
		irmo_client_callback_raise(client->disconnect_callbacks,
					   client);
	}
	
	// pump new atoms to send window
	
	proto_pump_client(client);

	gettimeofday(&nowtime, NULL);
	
	irmo_timeval_from_ms(timeout_length_ms, &timeout_length);

	//printf("timeout for client: %i ms (%i, %i)\n",
	//timeout_length_ms,
	//(int) client->rtt,
	//(int) client->rtt_deviation);
	
	irmo_timeval_sub(&nowtime, &timeout_length, &timeout_time);

	//printf("-- %i\n", timeout_time.tv_sec);

	//for (i=0; i<client->sendwindow_size; ++i) {
	//printf("%i: %i\n", i, client->sendwindow[i]->sendtime.tv_sec);
	//}

	for (i=0; i<client->sendwindow_size; ) {
		IrmoPacket *packet;
		int len;
		int start;

		//printf("find start\n");
		
		// search forward until we find the start of a block

		while (i<client->sendwindow_size
		       && irmo_timeval_cmp(&client->sendwindow[i]->sendtime,
					   &timeout_time) > 0) {
			//printf("atom %i not expired yet\n", i);
			++i;
		}

		// no more atoms?
		
		if (i >= client->sendwindow_size)
			break;

		//printf("find end\n");
		
		start = i;
		len = 0;
		
		while (i<client->sendwindow_size
		       && irmo_timeval_cmp(&client->sendwindow[i]->sendtime,
					   &timeout_time) <= 0
		       && len < PACKET_THRESHOLD) {
			//printf("atom %i out of date\n", i);
			len += client->sendwindow[i]->len;
			++i;
		}

		//printf("packet %i->%i\n", start, i-1);
		// build a packet 

		packet = proto_build_packet(client, start, i-1);

		//printf("sendpacket: %i->%i\n", start, i-1);
		
		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		//printf("free packet\n");
		
		// finished using packet
		
		packet_free(packet);
	}

	//printf("finished\n");

	// possibly we need to send an ack for something we have received
	// if we have nothing in our send window to send, we still need
	// to send an ack back

	if (client->need_ack) {
		IrmoPacket *packet;

		//printf("send ack to client\n");

		packet = packet_new(4);

		// only ack flag is sent, not dta as there is no data
		
		packet_writei16(packet, PACKET_FLAG_ACK);
		packet_writei16(packet, client->recvwindow_start & 0xffff);
		client->need_ack = FALSE;

		// send packet

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		packet_free(packet);
	}
}

// $Log$
// Revision 1.4  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.3  2003/08/18 01:23:14  fraggle
// Use G_INLINE_FUNC instead of inline for portable inline function support
//
// Revision 1.2  2003/07/22 02:32:34  fraggle
// Fix segfault in back-including past sendatoms
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.18  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.17  2003/05/19 13:41:08  sdh300
// Fix segfault in back-including of null atoms
//
// Revision 1.16  2003/05/08 08:20:05  sdh300
// In building packets, extend the range specified to include all null
// atoms before the range. This sends a small amount of redundant
// data which can make packet retransmission unneccesary.
//
// Revision 1.15  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.14  2003/04/21 18:10:54  sdh300
// Fix sending of unneccesary acks
// Slow start/Congestion avoidance
//
// Revision 1.13  2003/03/26 16:15:34  sdh300
// Disconnect clients after too long a timeout
//
// Revision 1.12  2003/03/21 17:21:45  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.11  2003/03/18 20:55:46  sdh300
// Initial round trip time measurement
//
// Revision 1.10  2003/03/16 20:18:19  sdh300
// Fix bug in packet building with run length encoding making runs too long
// (33 max rather than 32)
//
// Revision 1.9  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.8  2003/03/12 19:02:26  sdh300
// Comment out debug message
//
// Revision 1.7  2003/03/07 12:31:51  sdh300
// Add protocol.h
//
// Revision 1.6  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.5  2003/03/06 20:15:25  sdh300
// Initial ack code
//
// Revision 1.4  2003/03/05 17:33:57  sdh300
// Fix length threshold checking (len variable was uninitialised)
//
// Revision 1.3  2003/03/05 15:32:21  sdh300
// Add object class to change atoms to make their coding in packets
// unambiguous.
//
// Revision 1.2  2003/03/03 22:14:44  sdh300
// Initial window construction and sending of packets
//
// Revision 1.1  2003/03/02 02:12:28  sdh300
// Initial packet building code
//
