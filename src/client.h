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

#ifndef IRMO_INTERNAL_CLIENT_H
#define IRMO_INTERNAL_CLIENT_H

#include "public/client.h"

#include <netinet/in.h>

#include "sendatom.h"
#include "server.h"
#include "socket.h"
#include "universe.h"

// maximum sendwindow size

#define MAX_SENDWINDOW 1024

// size of each packet; when size of atoms passes this threshold
// no more atoms are added to it

#define PACKET_THRESHOLD 128

typedef enum {
	CLIENT_CONNECTING,          // received first syn, sent syn ack
	CLIENT_CONNECTED,           // received syn ack reply, normal op
	CLIENT_DISCONNECTED,        // killed with syn fin
	CLIENT_DISCONNECTING,       // waiting for ack of disconnect request 
} IrmoClientState;

// client

struct _IrmoClient {

	int refcount;
	
	IrmoClientState state;
	
	// server client is connected to

	IrmoServer *server;
	
	// clients remote universe
	
	IrmoUniverse *universe;

	// address:

	struct sockaddr *addr;

	// protocol stuff (internal)

	// time last syn/synack was sent
	time_t connect_time;
	gint connect_attempts;

	// when a client remotely disconnects, keep the client object
	// for several seconds before destroying it (if they do not
	// receive the disconnect ack they may send another disconnect
	// request, in which case another ack will need to be sent)
	
	gboolean disconnect_wait;

	// send queue 
	
	GQueue *sendq;

	// change entries in sendq are hashed by object id so
	// new changes can be added to the existing sendatoms
	
	GHashTable *sendq_hashtable;

	// position of start of send window in stream

	int sendwindow_start;
	
	// send window

	IrmoSendAtom *sendwindow[MAX_SENDWINDOW];
	int sendwindow_size;

	// receive window

	int recvwindow_start;

	IrmoSendAtom **recvwindow;
	int recvwindow_size;

	// if true, we need to send an ack to the client to acknowledge
	// something it has sent us
	
	gboolean need_ack;

	// disconnect callbacks

	GSList *disconnect_callbacks;

	// estimations of round trip time mean and standard deviation
	// (in milliseconds)

	float rtt;
	float rtt_deviation;

	// backoff multiply

	int backoff;

	// maximum send window size in bytes. once the amount of data
	// in the send window exceeds this amount, no more is added.

	float cwnd;

	// slow start threshold. when cwnd is above this threshold we
	// do congestion avoidance.

	int ssthresh;

	// user specified sendwindow limits
	// if these are 0, they are unset
	
	int local_sendwindow_max;
	int remote_sendwindow_max;
};

// create a new client, attached to a particular server

IrmoClient *irmo_client_new(IrmoServer *server, struct sockaddr *addr);

// 'run' a client, called by irmo_socket_run

void irmo_client_run(IrmoClient *client);

// destroy client data structure

void irmo_client_internal_unref(IrmoClient *client);

// run through sendatoms waiting in the receive window

void irmo_client_run_recvwindow(IrmoClient *client);

// timeout time for a client

int irmo_client_timeout_time(IrmoClient *client);

// preexec starting from a position in the recvwindow

void irmo_client_run_preexec(IrmoClient *client, int start, int end);

#endif /* #ifndef IRMO_INTERNAL_CLIENT_H */

// $Log$
// Revision 1.2  2003/08/30 03:08:00  fraggle
// Use irmo_client_internal_unref instead of unreffing externally. Make
// irmo_client_destroy static now.
//
// Revision 1.1.1.1  2003/06/09 21:33:23  fraggle
// Initial sourceforge import
//
// Revision 1.23  2003/06/09 21:06:50  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.22  2003/05/20 02:06:05  sdh300
// Add out-of-order execution of stream
//
// Revision 1.21  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.20  2003/04/25 00:17:28  sdh300
// Remove unneccesary leading underscores from variables in IrmoClient
//
// Revision 1.19  2003/04/21 18:10:53  sdh300
// Fix sending of unneccesary acks
// Slow start/Congestion avoidance
//
// Revision 1.18  2003/03/21 17:21:45  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.17  2003/03/17 17:34:27  sdh300
// Add disconnect callbacks for clients
//
// Revision 1.16  2003/03/17 16:48:22  sdh300
// Add ability to disconnect from servers and to disconnect clients
//
// Revision 1.15  2003/03/07 12:32:15  sdh300
// Add missing prototype and some documentation
//
// Revision 1.14  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.13  2003/03/06 20:15:24  sdh300
// Initial ack code
//
// Revision 1.12  2003/03/05 15:28:13  sdh300
// Add receive window and extra data for sendatoms in the receive window.
//
// Revision 1.11  2003/03/03 21:03:06  sdh300
// Initial packet building code
//
// Revision 1.10  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.9  2003/02/23 00:00:03  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.8  2003/02/20 18:24:59  sdh300
// Use GQueue instead of a GPtrArray for the send queue
// Initial change/destroy code
//
// Revision 1.7  2003/02/18 20:26:41  sdh300
// Initial send queue building/notification code
//
// Revision 1.6  2003/02/18 18:25:40  sdh300
// Initial queue object code
//
// Revision 1.5  2003/02/16 23:41:26  sdh300
// Reference counting for client and server objects
//
// Revision 1.4  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.3  2003/02/06 02:41:25  sdh300
// Add CLIENT_DISCONNECTED for disconnected clients
//
// Revision 1.2  2003/02/03 20:57:22  sdh300
// Initial client code
//
// Revision 1.1  2002/11/26 15:23:49  sdh300
// Initial network code
//

