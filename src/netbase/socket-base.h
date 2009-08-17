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

#ifndef IRMO_NETBASE_SOCKET_BASE_H
#define IRMO_NETBASE_SOCKET_BASE_H

#include <irmo/net-module.h>
#include "net-address.h"

// Sockets API headers:

#ifdef _WIN32

#include <WinSock.h>

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#define closesocket close

#endif

#define RECV_BUFFER_SIZE 2048

/*!
 * Implementation of a @ref IrmoNetSocket structure, for sockets-based
 * modules.
 */

typedef struct {

        /*! Parent structure */

        IrmoNetSocket irmo_socket;

        /*! The socket handle. */

        int sock;

        /*! Buffer into which to receive packets. */

        uint8_t *recvbuf;
} SockBaseSocket;

/*!
 * Open a socket of the specified socket family.
 *
 * @param module       The class for the new socket.
 * @param family       The sockets API family type.
 * @return             The new SockBaseSocket structure, or NULL if
 *                     it was not possible to create the new socket.
 */

SockBaseSocket *irmo_sockbase_open(IrmoNetSocketClass *socket_class,
                                   int family);

/*!
 * Close the specified socket.
 *
 * @param _sock        The socket to close.
 */

void irmo_sockbase_close(IrmoNetSocket *_sock);

/*!
 * Block on the specified sockets until a packet is received, or
 * until the timeout is reached.
 *
 * @param handles       Array of handles to block on.
 * @param num_handles   Number of handles in the array.
 * @param timeout       Number of milliseconds to wait before timeout.
 * @return              Non-zero for success.
 */

int irmo_sockbase_block(IrmoNetSocket **handles, int num_handles, int timeout);

#endif /* #ifndef IRMO_NETBASE_SOCKET_BASE_H */

