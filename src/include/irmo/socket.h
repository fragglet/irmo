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

#ifndef IRMO_SOCKET_H
#define IRMO_SOCKET_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * Socket objects listen on a particular UDP port. Servers (see
 * \ref IrmoServer) can then be attached to the socket to allow
 * connections. Multiple servers can listen on the same socket.
 * Connecting clients will be connected to a different server
 * depending on the hostname they specify. It is therefore 
 * possible to set up a "virtual host" system through this
 * mechanism.
 *
 * \addtogroup socket
 * \{
 */

/*!
 * Add a reference to a socket.
 *
 * \param sock  Socket to reference.
 * \sa irmo_socket_unref
 *
 */

void irmo_socket_ref(IrmoSocket *sock);

/*!
 * Remove a reference to a socket.
 *
 * The number of references to a socket is counted. The count starts at
 * 1. When the count reaches 0, the socket is destroyed.
 *
 * \param sock  Socket to unreference.
 * \sa irmo_socket_ref
 */

void irmo_socket_unref(IrmoSocket *sock);

/*!
 * Check for new packets received by a socket.
 *
 * This function must be called periodically to check for new packets
 * received by the socket and send new packets required by the network
 * protocol.
 *
 * \param sock   The socket to update.
 */

void irmo_socket_run(IrmoSocket *sock);

/*! 
 * Block on a set of sockets.
 *
 * This function blocks on an array of sockets, returning when data 
 * arrives at any of the sockets.
 *
 * \param 	sockets		An array of sockets to block on.
 * \param	nsockets	Number of sockets in the array.
 * \param 	timeout		Maximum time to block for, in milliseconds.
 * 				Specify 0 to block forever.
 */

void irmo_socket_block_set(IrmoSocket **sockets, int nsockets, int timeout);

/*!
 * Block on a single socket.
 *
 * This is the same as \ref irmo_socket_block, except it blocks on
 * a single socket.
 *
 * \param 	sock  		The socket to block on.
 * \param	timeout		Maximum time to block for, in milliseconds,
 * 				Specify 0 to block forever.
 */

void irmo_socket_block(IrmoSocket *sock, int timeout);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_SOCKET_H */

