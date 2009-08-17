//
// Copyright (C) 2009 Simon Howard
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

//
// Common code for sockets-based modules (IPv4/6)
//

#include "arch/sysheaders.h"
#include "base/error.h"
#include "base/alloc.h"

#include "algo/algo.h"

#include <irmo/module_ip.h>
#include <irmo/net-module.h>
#include "net-address.h"

#include "socket-base.h"

// Open a non-blocking socket.

SockBaseSocket *irmo_sockbase_open(IrmoNetSocketClass *socket_class,
                                   int family)
{
        SockBaseSocket *result;
        int sock;
        int opts;

        // Open the socket

        sock = socket(family, SOCK_DGRAM, 0);

        if (sock < 0) {
                return NULL;
        }

        // Set non-blocking mode:

#ifdef _WIN32
        // this is how we set nonblocking under windows
        {
                int trueval=1;
                ioctlsocket(sock, FIONBIO, &trueval);
        }
#else
        // this is how we set nonblocking under unix

        opts = fcntl(sock, F_GETFL);

        if (opts < 0) {
                irmo_error_report("irmo_sockbase_block"
                                  "cannot make socket nonblocking (%s)",
                                  strerror(errno));
                closesocket(sock);
                return NULL;
        }

        opts |= O_NONBLOCK;

        if (fcntl(sock, F_SETFL, opts) < 0) {
                irmo_error_report("irmo_sockbase_block"
                                  "cannot make socket nonblocking (%s)",
                                  strerror(errno));
                closesocket(sock);
                return NULL;
        }
#endif

        // Create a socket structure

        result = irmo_new0(SockBaseSocket, 1);
        result->irmo_socket.socket_class = socket_class;
        result->sock = sock;
        result->recvbuf = malloc(RECV_BUFFER_SIZE);

        return result;
}

// Close a socket.

void irmo_sockbase_close(IrmoNetSocket *_sock)
{
        SockBaseSocket *sock;

        sock = (SockBaseSocket *) _sock;

        closesocket(sock->sock);
        free(sock->recvbuf);
        free(sock);
}

// Block on a collection of sockets until new data is received or
// the specified timeout expires.

int irmo_sockbase_block(IrmoNetSocket **handles, int num_handles, int timeout)
{
        fd_set socket_set;
        SockBaseSocket *sock;
        struct timeval timeout_tv;
        struct timeval *timeout_param;
        int i;
        int max_sock;
        int result;

        // Build socket set and calculate the maximum socket value
 
        FD_ZERO(&socket_set);

        max_sock = 0;

        for (i=0; i<num_handles; ++i) {
                sock = (SockBaseSocket *) handles[i];

                FD_SET(sock->sock, &socket_set);

                if (sock->sock > max_sock) {
                        max_sock = sock->sock;
                }
        }

        // Calculate timeout_param

        if (timeout == 0) {
                timeout_param = NULL;
        } else {
                timeout_param = &timeout_tv;
                timeout_tv.tv_sec = timeout / 1000;
                timeout_tv.tv_usec = timeout % 1000;
        }

        result = select(max_sock + 1, &socket_set, NULL, NULL, timeout_param);

        if (result < 0) {
                perror("irmo_sockbase_block");
                return 0;
        } else {
                return 1;
        }
}

