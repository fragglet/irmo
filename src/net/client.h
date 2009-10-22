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

        // Number of references to this client.

	int refcount;

        // Current connection state of the client.

	IrmoClientState state;

	// Server to which the client is connected.

	IrmoServer *server;

        // The ID of this client.  Each client of a server is assigned
        // a unique ID.

        unsigned int id;

	// Client's remote world.

	IrmoWorld *world;

	// The network address used to send data to the client.

        IrmoNetAddress *address;

	// Time that the last syn/synack packet was sent.

	unsigned int connect_time;
	int connect_attempts;

        // If true, the client is in the disconnect wait state.
	// When a client remotely disconnects, the client object is
        // kept for several seconds before the client structure
        // is freed. This is because if the client does not receive
        // the disconnect ack, they may send another disconnect
        // request, in which case another ack will need to be sent.

	int disconnect_wait;

	// Send queue.  This is a queue of atoms waiting to be
        // added to the send window, for transmission to the
        // client.

	IrmoQueue *sendq;

        // Send queue lookup table.
	// Change atoms in the send queue are stored in this
        // table, hashed by object ID, so that new changes can be
        // added to atoms if a change is made to an object for
        // which an entry is already in the queue.

	IrmoHashTable *sendq_hashtable;

	// Sequence number of the first atom in the send window.

	unsigned int sendwindow_start;

	// Send window.  These are send atoms that have been sent
        // to the client, and are awaiting acknowldegement from
        // the client.

	IrmoSendAtom *sendwindow[MAX_SENDWINDOW];
	unsigned int sendwindow_size;

	// Sequence number of the first atom in the receive window.

	unsigned int recvwindow_start;

        // The receive window.  These are atoms that have been
        // received from the remote client, but not yet executed
        // (generally because of a lost packet earlier in sequence)

	IrmoSendAtom **recvwindow;
	unsigned int recvwindow_size;

	// If true, we need to send an ack to the client to acknowledge
	// something it has sent us.

	int need_ack;

	// Callbacks invoked when the client state changes,
        // a separate list for each state.

	IrmoCallbackList state_change_callbacks[IRMO_CLIENT_NUM_STATES];

	// Estimations of round trip time mean and standard deviation
	// (in milliseconds), for calculation of the send window size.

	float rtt;
	float rtt_deviation;

	// Resend backoff factor.  Each time a packet is resent, the
        // time before the next resend is doubled.
        // TODO: Don't do this?

	unsigned int backoff;

	// Maximum send window size in bytes. Once the amount of data
	// in the send window exceeds this amount, no more atoms are
        // added to the send window.

	float cwnd;

	// Slow start threshold. When cwnd is above this threshold we
	// do congestion avoidance.

	unsigned int ssthresh;

        // If true, the remote world (that the client is sharing to us)
        // has been synced to us. (ie. we have received the sync point
        // atom)

        int local_synced;

        // If true, the local world (that we are sharing to the client)
        // is synced to the client.  (ie. the client has acknowledged the
        // sync point atom)

        int remote_synced;

	// User specified sendwindow limits
	// If these are 0, they are unset, and automatic congestion
        // avoidance is used.

	unsigned int local_sendwindow_max;
	unsigned int remote_sendwindow_max;

	// String describing the reason why connection to a remote
        // server failed.

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

