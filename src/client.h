// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
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
//---------------------------------------------------------------------

#ifndef IRMO_INTERNAL_CLIENT_H
#define IRMO_INTERNAL_CLIENT_H

#include "public/client.h"

#include "netlib.h"
#include "sendatom.h"
#include "server.h"
#include "socket.h"
#include "world.h"

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
	
	// clients remote world
	
	IrmoWorld *world;

	// address:

	struct sockaddr *addr;

	// protocol stuff (internal)

	// time last syn/synack was sent
	time_t connect_time;
	int connect_attempts;

	// when a client remotely disconnects, keep the client object
	// for several seconds before destroying it (if they do not
	// receive the disconnect ack they may send another disconnect
	// request, in which case another ack will need to be sent)
	
	int disconnect_wait;

	// send queue 
	
	IrmoQueue *sendq;

	// change entries in sendq are hashed by object id so
	// new changes can be added to the existing sendatoms
	
	IrmoHashTable *sendq_hashtable;

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
	
	int need_ack;

	// disconnect callbacks

	IrmoSListEntry *disconnect_callbacks;

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

	// connection error

	char *connection_error;
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

