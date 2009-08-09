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

#ifndef IRMO_CONNECTION_H
#define IRMO_CONNECTION_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * This is the client side interface. @ref IrmoConnection objects
 * represent an open connection to a remote @ref IrmoServer.
 * The key function is @ref irmo_connect which establishes a new
 * connection.
 * 
 * In using irmo_connect, the 'iface' object is used to specify the
 * world being served by the remote server. The interface
 * file must be identical; if the interfaces differ
 * on the client and the server, the server will refuse the connection.
 *
 * The client may serve a world back to the server by setting
 * the 'local_world' parameter to irmo_connect to point to a
 * valid IrmoWorld object. The type here must also be identical to
 * that expected by the server.
 *
 * @addtogroup connection
 * \{
 */

/*!
 * Establish a new connection to a server.
 *
 * This establishes a new connection to a server on a remote machine.
 * If successful, an IrmoConnection object is returned representing
 * the connection to the server.
 *
 * Care must be taken with the interface and the local 
 * world: their types must match those expected by the server.
 * (see @ref irmo_server_new)
 *
 * @param net_module     Network module to use for communications.  For
 *                       IPv4, use @ref IRMO_NET_IPV4.
 * @param location       The hostname of the remote machine to connect to.
 * @param port           The port on the remote machine on which the server
 *                       is running.
 * @param iface          An interface describing the remote world served by
 *                       the server, or NULL if the server is not serving
 *                       a world.
 * @param local_world    A local world to serve back to the server, or
 *                       NULL not to serve any world back.
 * @return               An @ref IrmoConnection object representing the
 *                       connection, or NULL if a connection could not be
 *                       established.
 */

IrmoConnection *irmo_connect(IrmoNetModule *net_module,
                             char *location,
                             unsigned int port,
                             IrmoInterface *iface,
                             IrmoWorld *local_world);

/*!
 * Close a connection to a remote server.
 *
 * This closes a connection to a remote server. It blocks until the
 * server acknowledges the disconnect request or the connection
 * times out.
 *
 * Note that this does not destroy the @ref IrmoConnection object, only
 * closes the connection. To destroy the object, use the 
 * @ref irmo_connection_unref function.
 *
 * @param conn     The connection to close.
 *
 */

void irmo_disconnect(IrmoConnection *conn);

/*!
 * Check for new packets received from a server.
 *
 * This function must be called periodically to check for new packets
 * received from the server and to send new packets required by the
 * network protocol.
 * 
 * @param conn      The connection.
 */

void irmo_connection_run(IrmoConnection *conn);

/*!
 * Block on a connection until a new packet is received.
 *
 * @param conn      The connection.
 * @param timeout   Maximum time to wait (in MS) before returning.
 */

void irmo_connection_block(IrmoConnection *conn, int timeout);

/*!
 * Get the world object for a remote server.
 *
 * Servers can serve a @ref IrmoWorld object to their connecting
 * clients. This function returns a reference to the local copy of 
 * the remote world (or NULL if the server is not serving a
 * world)
 *
 * @sa irmo_server_new
 * @sa irmo_client_get_world
 */

IrmoWorld *irmo_connection_get_world(IrmoConnection *conn);

/*!
 * Get the state of a connection.
 *
 * @param conn              The connection.
 * @return                  Current state of the connection.
 */

IrmoClientState irmo_connection_get_state(IrmoConnection *conn);

/*!
 * Add a reference to an @ref IrmoConnection object.
 */

void irmo_connection_ref(IrmoConnection *conn);

/*!
 * Remove a reference to an @ref IrmoConnection object.
 */

void irmo_connection_unref(IrmoConnection *conn);

#ifdef __cplusplus
}
#endif

//! \}

#endif /* #ifndef IRMO_CONNECTION_H */

