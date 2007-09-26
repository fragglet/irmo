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

#ifndef IRMO_SERVER_H
#define IRMO_SERVER_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * Server objects are used to serve a particular \ref IrmoWorld
 * world object on an \ref IrmoSocket socket. Clients can then
 * connect to the server using the \ref irmo_connect function
 * and receive information about the world being served.
 * 
 * The clients themselves may also serve a world back to the 
 * server. The client world to serve is specified using the
 * 'spec' field of the irmo_server_new constructor.
 *
 * \addtogroup server
 * \{
 */

/*!
 * \brief Create a new server
 *
 * Create a new \ref IrmoServer listening for connections on a
 * particular port.
 *
 * \param domain      The type of socket to listen on 
 *                    (see \ref IrmoSocketDomain)
 * \param port        The port number to listen on
 * \param world       The world the server will serve.
 * \param client_spec An interface specification describing the interface
 *                    for the client world of the connecting clients.
 * \return            A new IrmoServer or NULL if the function fails.
 */

IrmoServer *irmo_server_new(IrmoSocketDomain domain, int port,
			    IrmoWorld *world, IrmoInterface *client_spec);

/*!
 * \brief Watch new connections to a server
 *
 * Add a new watch on a server. Whenever a new client connects to
 * the server, a callback function will be invoked.
 *
 * \param server     The server to watch
 * \param func       The function to call
 * \param user_data  Some extra data to pass to the callback function
 *
 * \return           A \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_server_watch_connect(IrmoServer *server, 
					IrmoClientCallback func,
					void *user_data);


/*!
 * Iterate over all clients connected to a server
 * 
 * Calls a specified function for each client currently connected to
 * the given server.
 *
 * \param server	The server to iterate over
 * \param func		The function to call
 * \param user_data	Extra data to pass to the function
 *
 */

void irmo_server_foreach_client(IrmoServer *server,
				IrmoClientCallback func,
				void *user_data);

/*!
 * \brief Reference a server object
 *
 * Add a reference to a server object. When a server is created its 
 * reference count is set to 1. References can be added with 
 * irmo_server_ref and removed with \ref irmo_server_unref.
 * When the reference count reaches
 * 0 the server is automatically shut down and destroyed.
 *
 * \param server   The server object to reference.
 * \sa irmo_server_unref
 *
 */

void irmo_server_ref(IrmoServer *server);

/*!
 * \brief  Unreference a server object
 * 
 * \param server   The server object to unreference
 * \sa irmo_server_ref
 *
 */

void irmo_server_unref(IrmoServer *server);

/*!
 * \brief Shutdown a server
 *
 * This shuts down a server, blocking until all clients are disconnected.
 *
 * \param server	The server to shutdown
 */

void irmo_server_shutdown(IrmoServer *server);

/*! 
 * \brief Get the socket used by a server
 *
 * Returns a reference to the \ref IrmoSocket socket object being
 * used by a server.
 *
 */

IrmoSocket *irmo_server_get_socket(IrmoServer *server);

/*!
 * \brief Check for new packets received at the server.
 *
 * This function must be called periodically to check for new packets
 * received and send new packets required by the network
 * protocol.
 * 
 * This is identical to:
 *   irmo_socket_run(irmo_server_get_socket(conn));
 *
 * \sa irmo_socket_run
 */

void irmo_server_run(IrmoServer *server);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_SERVER_H */

