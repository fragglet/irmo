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

#ifndef IRMO_CONNECTION_H
#define IRMO_CONNECTION_H

/*!
 * \addtogroup connection
 * \{
 */

#include "if_spec.h"
#include "client.h"
#include "server.h"
#include "universe.h"

/*!
 * \brief An IrmoConnection object 
 *
 * This represents a connection to a \ref IrmoServer server running
 * on a remote machine.
 *
 * This is exactly the same as an \ref IrmoClient. Internally the remote
 * server is seen as a client connected back to the local machine.
 */

typedef IrmoClient IrmoConnection;

/*!
 * \brief Establish a new connection to a server
 *
 * This establishes a new connection to a server on a remote machine.
 * If successful, an IrmoConnection object is returned representing
 * the connection to the server.
 *
 * Care must be taken in the interface specification and the local 
 * universe: their types must match those expected by the server.
 * (see \ref irmo_server_new)
 *
 * \param domain         Domain of the socket to use. Usually this will
 *                       be AF_INET.
 * \param location       The hostname of the remote machine to connect to.
 * \param port           The port on the remote machine on which the server
 *                       is running.
 * \param spec           An interface specification object describing the
 *                       remove universe served by the server, or NULL if
 *                       the server is not serving a universe.
 * \param local_universe A local universe to serve back to the server, or
 *                       NULL not to serve any universe back.
 * \return               A IrmoConnection object representing the
 *                       connection, or NULL if a connection could not be
 *                       established.
 */

IrmoConnection *irmo_connect(int domain, gchar *location, int port,
                             IrmoInterfaceSpec *spec,
			     IrmoUniverse *local_universe);

/*!
 * \brief Close a connection to a remote server
 *
 * This closes a connection to a remote server. It blocks until the
 * server acknowledges the disconnect request or the connection
 * times out.
 *
 * Note that this does not destroy the \ref IrmoConnection object, only
 * closes the connection. To destroy the object, use the 
 * \ref irmo_connection_unref function.
 *
 * \param conn     The connection to close
 *
 */

void irmo_disconnect(IrmoConnection *conn);

/*!
 * \brief Get the socket used by a IrmoConnection
 * 
 * Returns the socket object being used by a connection for network
 * communications.
 *
 */

IrmoSocket *irmo_connection_get_socket(IrmoConnection *conn);

/*!
 * \brief Check for new packets received from the server.
 *
 * This function must be called periodically to check for new packets
 * received socket and send new packets required by the network
 * protocol.
 * 
 * This is identical to:
 *   irmo_socket_run(irmo_connection_get_socket(conn));
 *
 * \sa irmo_socket_run
 */

void irmo_connection_run(IrmoConnection *conn);

/*!
 * \brief Get the universe object for the remote server
 *
 * Servers can serve a \ref IrmoUniverse object to their connecting
 * clients. This function returns a reference to the local copy of 
 * the remote universe (or NULL if the server is not serving a
 * universe)
 *
 * \sa irmo_server_new
 * \sa client_get_universe
 */

IrmoUniverse *irmo_connection_get_universe(IrmoConnection *conn);

#define irmo_connection_ref irmo_client_ref
#define irmo_connection_unref irmo_client_unref

//! \}

#endif /* #ifndef IRMO_CONNECTION_H */

// $Log$
// Revision 1.1  2003/06/09 21:33:25  fraggle
// Initial revision
//
// Revision 1.9  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.8  2003/03/17 16:48:27  sdh300
// Add ability to disconnect from servers and to disconnect clients
//
// Revision 1.7  2003/03/17 15:45:38  sdh300
// Remove 'IrmoConnection' object; make it into a typedef for IrmoClient
//
// Revision 1.6  2003/03/07 12:17:22  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.5  2003/03/07 10:48:07  sdh300
// Add new sections to documentation
//
// Revision 1.4  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.3  2003/02/23 00:45:43  sdh300
// Add universe access functions for client, connection
//
// Revision 1.2  2003/02/23 00:26:57  sdh300
// Add some documentation and helper functions for IrmoConnection
//
// Revision 1.1  2003/02/23 00:00:06  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.2  2003/02/06 02:10:11  sdh300
// Add missing CVS tags
//
