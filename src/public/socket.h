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

#ifndef IRMO_SOCKET_H
#define IRMO_SOCKET_H

/*!
 * \addtogroup socket
 * \{
 */


/*!
 * \brief an IrmoSocket object.
 */

typedef struct _IrmoSocket IrmoSocket;

#include "client.h"
#include "server.h"

/*!
 * \brief Socket domain
 * 
 * When creating a new socket, it is neccessary to specify the type
 * of socket to be created, IPv4 or IPv6.
 */

typedef enum {
	IRMO_SOCKET_AUTO,
	IRMO_SOCKET_IPV4,
	IRMO_SOCKET_IPV6,
} IrmoSocketDomain;

/*!
 * \brief Create a new socket
 *
 * Create a new socket listening on a particular port. Servers can then
 * be attached to the socket to serve worlds on different vhosts.
 *
 * \param domain  The type of socket to create (see \ref IrmoSocketDomain)
 * \param port    The port number to listen on
 * \return        A new IrmoSocket or NULL if the new socket could
 *                not be created.
 */

IrmoSocket *irmo_socket_new(IrmoSocketDomain domain, int port);

/*!
 * \brief Add a reference to a socket
 *
 * \param sock  Socket to reference
 * \sa irmo_socket_unref
 *
 */

void irmo_socket_ref(IrmoSocket *sock);

/*!
 * \brief Remove a reference to a socket
 *
 * The number of references to a socket is counted. The count starts at
 * 1. When the count reaches 0, the socket is destroyed.
 *
 * \param sock  Socket to unreference
 * \sa irmo_socket_ref
 */

void irmo_socket_unref(IrmoSocket *sock);

/*!
 * \brief Run socket
 *
 * This function must be called periodically to check for new packets
 * received by the socket and send new packets required by the network
 * protocol.
 *
 * \param sock   The socket to update
 */

void irmo_socket_run(IrmoSocket *sock);

/*! 
 * \brief Block on a set of sockets
 *
 * This function blocks on an array of sockets, returning when data 
 * arrives at any of the sockets.
 *
 * \param 	sockets		An array of sockets to block on
 * \param	nsockets	Number of sockets in the array
 * \param 	timeout		Maximum time to block for, in milliseconds.
 * 				Specify 0 to block forever.
 */

void irmo_socket_block_set(IrmoSocket **sockets, int nsockets, int timeout);

/*!
 * \brief Block on a single socket
 *
 * This is the same as \ref irmo_socket_block, except it blocks on
 * a single socket.
 *
 * \param 	socket		The socket to block on
 * \param	timeout		Maximum time to block for, in milliseconds,
 * 				Specify 0 to block forever.
 */

void irmo_socket_block(IrmoSocket *sock, int timeout);

//! \}

#endif /* #ifndef IRMO_SOCKET_H */

// $Log$
// Revision 1.7  2003/09/01 18:59:28  fraggle
// Add a timeout parameter for blocking on sockets. Use block function
// internally.
//
// Revision 1.6  2003/09/01 18:52:51  fraggle
// Blocking functions for IrmoSocket
//
// Revision 1.5  2003/09/01 17:03:39  fraggle
// Fix documentation
//
// Revision 1.4  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.3  2003/08/26 14:57:31  fraggle
// Remove AF_* BSD sockets dependency from Irmo API
//
// Revision 1.2  2003/08/16 18:13:48  fraggle
// Remove dependency on BSD sockets API in Irmo API
//
// Revision 1.1.1.1  2003/06/09 21:33:26  fraggle
// Initial sourceforge import
//
// Revision 1.4  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.3  2003/03/07 12:17:22  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.2  2003/03/07 10:48:08  sdh300
// Add new sections to documentation
//
// Revision 1.1  2003/02/23 00:00:07  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.6  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.5  2003/02/11 17:53:01  sdh300
// Add socket_run and documentation (missing from the header)
//
// Revision 1.4  2003/02/06 00:59:26  sdh300
// Split up socket constructor function, so that unbound sockets can
// be created for clients
//
// Revision 1.3  2002/12/02 21:32:51  sdh300
// reference counting for IrmoSockets
//
// Revision 1.2  2002/11/26 15:32:12  sdh300
// store port number in IrmoSocket objects as well
//
// Revision 1.1  2002/11/26 15:23:50  sdh300
// Initial network code
//
