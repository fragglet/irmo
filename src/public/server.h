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

#ifndef IRMO_SERVER_H
#define IRMO_SERVER_H

/*!
 * \addtogroup server
 * \{
 */

/*!
 * \brief An IrmoServer object
 */

typedef struct _IrmoServer IrmoServer;

#include "if_spec.h"
#include "client.h"
#include "socket.h"
#include "universe.h"

/*!
 * \brief Create a new server
 *
 * Create a new \ref IrmoServer listening for connections on a
 * particular vhost.
 *
 * \param sock      The socket to listen on.
 * \param hostname  The vhost hostname to listen on. If NULL is passed
 *                  for this parameter, this server will be the default
 *                  server for the socket. If there is an existing
 *                  default server, the function will fail.
 * \param universe  The universe the server will serve.
 * \param spec      An interface specification describing the interface
 *                  for the client universe of the connecting clients.
 * \return          A new IrmoServer or NULL if the function fails.
 */

IrmoServer *irmo_server_new(IrmoSocket *sock, gchar *hostname,
			    IrmoUniverse *universe, IrmoInterfaceSpec *spec);

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
					gpointer user_data);


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
				gpointer user_data);

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

//! \}

#endif /* #ifndef IRMO_SERVER_H */

// $Log$
// Revision 1.3  2003/08/15 15:22:42  fraggle
// Add an iterator function to iterate over clients connected to a server.
//
// Revision 1.2  2003/07/22 02:05:40  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:26  fraggle
// Initial sourceforge import
//
// Revision 1.8  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.7  2003/03/17 17:35:15  sdh300
// Add disconnect callbacks for clients
// Move IrmoClientCallback to client.h
//
// Revision 1.6  2003/03/07 14:31:23  sdh300
// Callback functions for watching new client connects
//
// Revision 1.5  2003/03/07 12:17:22  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.4  2003/03/07 10:48:08  sdh300
// Add new sections to documentation
//
// Revision 1.3  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.2  2003/02/23 00:35:16  sdh300
// Add some missing documentation
//
// Revision 1.1  2003/02/23 00:00:07  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.4  2003/02/16 23:41:27  sdh300
// Reference counting for client and server objects
//
// Revision 1.3  2002/11/26 15:46:41  sdh300
// Fix compile and possible namespace conflicts with the "socket" function
//
// Revision 1.2  2002/11/26 15:43:05  sdh300
// Initial IrmoServer code
//
// Revision 1.1  2002/11/26 15:23:49  sdh300
// Initial network code
//

