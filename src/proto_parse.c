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

// parsing of received packets
// this code runs on the assumption that the packets being given to
// it are well formed and valid; it does not do any checking on the data
// received.
//
// TODO: proto_verify.c for verifification of packets before they are
// parsed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet.h"
#include "protocol.h"
#include "sendatom.h"

// alpha value used for estimating round trip time

#define RTT_ALPHA 0.9

gboolean irmo_proto_use_preexec = TRUE;

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
			= g_renew(IrmoSendAtom *,
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
	guint i;
	int seq;
	int start;
	
	// get the start position
	
	irmo_packet_readi16(packet, &i);

	start = get_stream_position(client->recvwindow_start, i);

	//printf("stream position: %i->%i\n", i, start);
	
	for (seq=start;;) {
		IrmoSendAtomClass *klass;
		IrmoSendAtomType atomtype;
		int natoms;
		guint byte;
		
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
				client->need_ack = TRUE;
			
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
		GTimeVal nowtime, rtt;
		int rtt_ms, deviation;
		
		g_get_current_time(&nowtime);

		irmo_timeval_sub(&nowtime, &client->sendwindow[0]->sendtime,
				 &rtt);

		rtt_ms = irmo_timeval_to_ms(&rtt);

		deviation = abs(rtt_ms - client->rtt);
		
		client->rtt = RTT_ALPHA * client->rtt
			+ (1-RTT_ALPHA) * rtt_ms;

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
		guint i;

		irmo_packet_readi16(packet, &i);

		proto_parse_ack(client, i);
	}

	if (packet->flags & PACKET_FLAG_DTA) {
		proto_parse_packet_cluster(client, packet);

		irmo_client_run_recvwindow(client);
	}
}

// $Log$
// Revision 1.12  2003/11/18 19:32:19  fraggle
// Use GTimeVal instead of struct timeval
//
// Revision 1.11  2003/10/14 22:12:50  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.10  2003/10/14 00:53:43  fraggle
// Remove pointless inlinings
//
// Revision 1.9  2003/09/12 11:30:26  fraggle
// Rename IrmoVarType to IrmoValueType to be orthogonal to IrmoValue
//
// Revision 1.8  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.7  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.6  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.5  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.4  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.3  2003/08/18 01:23:14  fraggle
// Use G_INLINE_FUNC instead of inline for portable inline function support
//
// Revision 1.2  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.18  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.17  2003/05/21 02:20:36  sdh300
// Add ability to disable preexec
//
// Revision 1.16  2003/05/20 02:06:06  sdh300
// Add out-of-order execution of stream
//
// Revision 1.15  2003/05/07 15:48:05  sdh300
// Fix backoff timeout bug
//
// Revision 1.14  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.13  2003/04/21 18:10:54  sdh300
// Fix sending of unneccesary acks
// Slow start/Congestion avoidance
//
// Revision 1.12  2003/03/21 17:21:45  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.11  2003/03/18 20:55:46  sdh300
// Initial round trip time measurement
//
// Revision 1.10  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.9  2003/03/14 01:07:23  sdh300
// Initial packet verification code
//
// Revision 1.8  2003/03/12 18:59:26  sdh300
// Remove/comment out some debug messages
//
// Revision 1.7  2003/03/07 12:31:51  sdh300
// Add protocol.h
//
// Revision 1.6  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.5  2003/03/06 20:42:06  sdh300
// Fix a couple of bugs in received change atom creation
//
// Revision 1.4  2003/03/06 20:15:25  sdh300
// Initial ack code
//
// Revision 1.3  2003/03/06 19:23:14  sdh300
// Add initial code to run through the atoms in the send window
//
// Revision 1.2  2003/03/06 18:08:14  sdh300
// Add missing cvs log tag
//
