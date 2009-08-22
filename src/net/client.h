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

#ifndef IRMO_NET_CLIENT_H
#define IRMO_NET_CLIENT_H

#include <irmo/client.h>

#include "netbase/net-address.h"
#include "world/world.h"

#include "sendatom.h"
#include "server.h"

// maximum sendwindow size

#define MAX_SENDWINDOW 1024

// maximum packet size: when a packet exceeds this,
// no more atoms are added to it

#define IRMO_PROTOCOL_MTU 1024

// client

struct _IrmoClient {

	int refcount;

	IrmoClientState state;

	// server client is connected to

	IrmoServer *server;

        // The ID of this client.  Each client of a server is assigned
        // a unique ID.

        unsigned int id;

	// client's remote world

	IrmoWorld *world;

	// address:

        IrmoNetAddress *address;

	// protocol stuff (internal)

	// time last syn/synack was sent
	unsigned int connect_time;
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

	unsigned int sendwindow_start;
	
	// send window

	IrmoSendAtom *sendwindow[MAX_SENDWINDOW];
	unsigned int sendwindow_size;

	// receive window

	unsigned int recvwindow_start;

	IrmoSendAtom **recvwindow;
	unsigned int recvwindow_size;

	// if true, we need to send an ack to the client to acknowledge
	// something it has sent us
	
	int need_ack;

	// Callbacks invoked when the client state changes,
        // a separate list for each state.

	IrmoCallbackList state_change_callbacks[IRMO_CLIENT_NUM_STATES];

	// estimations of round trip time mean and standard deviation
	// (in milliseconds)

	float rtt;
	float rtt_deviation;

	// backoff multiply

	unsigned int backoff;

	// maximum send window size in bytes. once the amount of data
	// in the send window exceeds this amount, no more is added.

	float cwnd;

	// slow start threshold. when cwnd is above this threshold we
	// do congestion avoidance.

	unsigned int ssthresh;

        // if true, the remote world (that the client is sharing to us)
        // has been synced to us. (ie. we have received the sync point
        // atom)

        int local_synced;

        // if true, the local world (that we are sharing to the client)
        // is synced to the client.  (ie. the client has received the
        // sync point atom)

        int remote_synced;

	// user specified sendwindow limits
	// if these are 0, they are unset

	unsigned int local_sendwindow_max;
	unsigned int remote_sendwindow_max;

	// connection error

	char *connection_error;
};

/*!
 * Allocate a new @ref IrmoClient, attached to a particular @ref IrmoServer.
 *
 * @param server         The server that the client is attached to.
 * @param address        Network address of the server.
 * @return               A new @ref IrmoClient.
 */

IrmoClient *irmo_client_new(IrmoServer *server, IrmoNetAddress *address);

/*!
 * Remove an internal reference to a @ref IrmoClient.
 *
 * @param client         The client.
 */

void irmo_client_internal_unref(IrmoClient *client);

/*!
 * Run through actions that a client needs to do.  This is called periodically
 * to update clients.
 *
 * @param client         The client.
 */

void irmo_client_run(IrmoClient *client);

/*!
 * Calculate the timeout time for the specified client, ie. the number of
 * milliseconds after which a sent atom is judged to have timed out.
 *
 * @param client         The client.
 * @return               Timeout time, in ms.
 */

unsigned int irmo_client_timeout_time(IrmoClient *client);

/*!
 * Run through send atoms waiting in a client's receive window.
 *
 * @param client         The client.
 */

void irmo_client_run_recvwindow(IrmoClient *client);

/*!
 * Run through send atoms waiting in a client's receive window, executing
 * them before they can be "officially" run, if possible.
 *
 * @param client         The client.
 * @param start          Start of the range of atoms to execute.
 * @param end            End of the range of atoms to execute.
 */

void irmo_client_run_preexec(IrmoClient *client, unsigned int start,
                             unsigned int end);

/*!
 * Set the connection state of a client.
 *
 * @param client         The client.
 * @param state          The state to set.
 */

void irmo_client_set_state(IrmoClient *client, IrmoClientState state);

#endif /* #ifndef IRMO_NET_CLIENT_H */

