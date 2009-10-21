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

#ifndef IRMO_SERVER_H
#define IRMO_SERVER_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * Server objects are used to serve a particular @ref IrmoWorld
 * world object over a network connection. Clients can then
 * connect to the server using the @ref irmo_connect function
 * and receive information about the world being served.
 * 
 * The clients themselves may also serve a world back to the 
 * server. The client world to serve is specified using the
 * 'client_interface' field of the irmo_server_new constructor.
 *
 * @addtogroup server
 * \{
 */

/*!
 * Create a new @ref IrmoServer listening for connections on a
 * particular port.
 *
 * @param net_module       Network module to use for communications.
 *                         For IPv4, use @ref IRMO_NET_IPV4.
 * @param port             The port number to listen on.
 * @param world            The world the server will serve.
 * @param client_interface An @ref IrmoInterface describing the 
 *                         client world of the connecting clients.
 * @return                 A new @ref IrmoServer or NULL if the
 *                         function failed.
 */

IrmoServer *irmo_server_new(IrmoNetModule *net_module,
                            unsigned int port,
			    IrmoWorld *world,
                            IrmoInterface *client_interface);

/*!
 * Watch new connections to a server.
 *
 * Whenever a new client connects to the server, a callback function
 * will be invoked.
 *
 * @param server     The server to watch.
 * @param func       The function to call.
 * @param user_data  Some extra data to pass to the callback function.
 *
 * @return           A @ref IrmoCallback object representing the watch.
 */

IrmoCallback *irmo_server_watch_connect(IrmoServer *server, 
					IrmoClientCallback func,
					void *user_data);

/*!
 * Iterate over all clients connected to a server.
 *
 * @param server        The server to iterate over.
 * @return              Pointer to an @ref IrmoIterator object to iterate
 *                      over all clients connected to the server.
 */

IrmoIterator *irmo_server_iterate_clients(IrmoServer *server);

/*!
 * Add a reference to a server object. When a server is created its 
 * reference count is set to 1. References can be added with 
 * irmo_server_ref and removed with @ref irmo_server_unref.
 * When the reference count reaches 0, the server is automatically
 * shut down and destroyed.
 *
 * @param server   The server object to reference.
 * @sa irmo_server_unref
 *
 */

void irmo_server_ref(IrmoServer *server);

/*!
 * Unreference a server object.
 * 
 * @param server   The server object to unreference.
 * @sa irmo_server_ref
 *
 */

void irmo_server_unref(IrmoServer *server);

/*!
 * Shutdown a server, blocking until all clients are disconnected.
 *
 * @param server	The server to shutdown.
 */

void irmo_server_shutdown(IrmoServer *server);

/*!
 * Check for new packets received by a server.
 *
 * This function must be called periodically to check for new packets
 * received and send new packets required by the network
 * protocol.
 * 
 * @param server        The server.
 */

void irmo_server_run(IrmoServer *server);

/*!
 * Block until new packets are received by a server.
 *
 * This function sleeps until @ref irmo_server_run can be run again.
 *
 * @param server        The server.
 * @param timeout       Maximum time to sleep for, in ms.
 */

void irmo_server_block(IrmoServer *server, int timeout);


//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_SERVER_H */

