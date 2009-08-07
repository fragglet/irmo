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

#ifndef IRMO_NET_SERVER_H
#define IRMO_NET_SERVER_H

#include <irmo/server.h>

#include "interface/interface.h"
#include "world/world.h"

#include "netbase/net-socket.h"

#include "client.h"

struct _IrmoServer {

	// reference count

	int refcount;

	// server running?

	int running;

        // Internal servers used as part of an IrmoConnection have this
        // set to true.

        int internal_server;

	// socket this server is using

	IrmoNetSocket *socket;

	// world being served by this server

	IrmoWorld *world;

	// interface for client worlds

	IrmoInterface *client_interface;

	// connected IrmoClients hashed by IP

	IrmoHashTable *clients;

        // connected IrmoClients hashed by ID:

        IrmoHashTable *clients_by_id;

	// callback functions for new connections

	IrmoCallbackList connect_callbacks;

        // The next value to try when assigning a new client ID.

        IrmoClientID next_id;

        // The remote client ID.  This is only used if this is an internal
        // server used as part of an IrmoConnection.  This holds the
        // unique client ID assigned to us by the remote server.

        IrmoClientID remote_client_id;
};

/*!
 * Internal function to create a new @ref IrmoServer.
 *
 * @param sock              The socket to use for the server.
 * @param world             The world that the server will serve.
 * @param client_interface  The interface for the client world.
 * @return                  The new server.
 */

IrmoServer *irmo_server_new_from(IrmoNetSocket *sock, IrmoWorld *world,
				 IrmoInterface *client_interface);

/*!
 * Get a new client ID that is not already in use.
 *
 * @param server           The server.
 * @return                 An unused identifier, for the new client.
 */

IrmoClientID irmo_server_assign_id(IrmoServer *server);

/*!
 * Invoke a list of @ref IrmoClientCallback callback functions.
 *
 * @param list             The list of callbacks to invoke.
 * @param client           The client to pass to the callback functions.
 */

void irmo_client_callback_raise(IrmoCallbackList *list, IrmoClient *client);

/*!
 * Invoke a list of callback functions when a client connects to a server.
 *
 * @param server           The server.
 * @param client           The client that connected to the server.
 */

void irmo_server_raise_connect(IrmoServer *server, IrmoClient *client);

#endif /* #ifndef IRMO_NET_SERVER_H */

