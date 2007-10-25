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

//
// Base socket code
//

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "netbase/netlib.h"
#include <irmo/packet.h>

#include "connection.h"
#include "protocol.h"
#include "socket.h"

// size of packet buffer (maximum packet size
// 64KiB default

#define PACKET_BUFFER_LEN 0x10000

// socket send function

void irmo_socket_sendpacket(IrmoSocket *sock, struct sockaddr *dest,
			    IrmoPacket *packet)
{
	int result;

	result = sendto(sock->sock,
			irmo_packet_get_buffer(packet),
			irmo_packet_get_length(packet),
			0,
			dest,
			irmo_sockaddr_len(dest->sa_family));

	if (result < 0) {
		irmo_error_report("irmo_socket_sendpacket",
				  "error sending packet (%s)",
				  strerror(errno));
	}
}

int irmo_socket_type_to_domain(IrmoSocketDomain type)
{
	switch (type) {
	case IRMO_SOCKET_AUTO:
		return AF_UNSPEC;
	case IRMO_SOCKET_IPV4:
		return AF_INET;
#ifdef USE_IPV6
	case IRMO_SOCKET_IPV6:
		return AF_INET6;
#endif
	default:
		return -1;
	}
}

// shared constructor
// this does not bind to a port

static IrmoSocket *_socket_new(IrmoSocketDomain type)
{
	IrmoSocket *irmosock;
	int sock;
	int domain;
	int opts;

	if (type == IRMO_SOCKET_AUTO)
		type = IRMO_SOCKET_IPV4;

	// check for supported domains

	if (type != IRMO_SOCKET_IPV4
#ifdef USE_IPV6
         && type != IRMO_SOCKET_IPV6
#endif
		) {
		irmo_error_report("irmo_socket_new",
				  "unsupported socket type(%i)", type);
		return NULL;
	}
	
	// try to create the socket and bind to the port

	domain = irmo_socket_type_to_domain(type);
	
	sock = socket(domain, SOCK_DGRAM, 0);

	if (sock < 0) {
		irmo_error_report("irmo_socket_new",
				  "cannot create new datagram socket (%s)",
				  strerror(errno));

		return NULL;
	}

	// make socket nonblocking

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
		irmo_error_report("irmo_socket_new"
				  "cannot make socket nonblocking (%s)",
				  strerror(errno));
		closesocket(sock);
		return NULL;
	}

	opts |= O_NONBLOCK;

	if (fcntl(sock, F_SETFL, opts) < 0) {
		irmo_error_report("irmo_socket_new"
				  "cannot make socket nonblocking (%s)",
				  strerror(errno));
		closesocket(sock);
		return NULL;
	}
#endif
	
	// wrap it all up in an IrmoSocket object

	irmosock = irmo_new0(IrmoSocket, 1);
	irmosock->refcount = 1;
	irmosock->domain = type;
	irmosock->sock = sock;

	return irmosock;
}

void irmo_socket_ref(IrmoSocket *sock)
{
	irmo_return_if_fail(sock != NULL);
	
	++sock->refcount;
}

void irmo_socket_shutdown(IrmoSocket *sock)
{
	irmo_return_if_fail(sock->shutdown != 1);
	
	// close socket

	closesocket(sock->sock);
		
	sock->shutdown = 1;
}

void irmo_socket_unref(IrmoSocket *sock)
{
	irmo_return_if_fail(sock != NULL);
	
	--sock->refcount;

	if (sock->refcount <= 0) {

		if (!sock->shutdown)
			irmo_socket_shutdown(sock);

		free(sock);
	}
}

// create a socket for clients, unbound

IrmoSocket *irmo_socket_new_unbound(IrmoSocketDomain type)
{
	IrmoSocket *sock = _socket_new(type);

	if (!sock)
		return NULL;

	sock->port = -1;
	
	return sock;
}

// create a socket for servers, bound to a port

IrmoSocket *irmo_socket_new_bound(IrmoSocketDomain domain, int port)
{
	IrmoSocket *sock;
	struct sockaddr *addr;
	int addr_len;
	int result;

	sock = _socket_new(domain);
	
	if (!sock)
		return NULL;
	
	// try to bind to the port

	addr_len = irmo_sockaddr_len(irmo_socket_type_to_domain(sock->domain));
	addr = (struct sockaddr *) irmo_malloc0(addr_len);

	switch (sock->domain) {
	case IRMO_SOCKET_IPV4:
		((struct sockaddr_in *) addr)->sin_family = AF_INET;
		((struct sockaddr_in *) addr)->sin_addr.s_addr = INADDR_ANY;
		((struct sockaddr_in *) addr)->sin_port = htons(port);
		break;
#ifdef USE_IPV6
	case IRMO_SOCKET_IPV6:
		((struct sockaddr_in6 *) addr)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *) addr)->sin6_addr = in6addr_any;
		((struct sockaddr_in6 *) addr)->sin6_port = htons(port);
		
		break;
#endif
        default:
                break;
	}
	
	result = bind(sock->sock, addr, addr_len);

	free(addr);

	if (result < 0) {
		irmo_error_report("irmo_socket_new",
				  "cannot bind socket to port %i (%s)",
				  port, strerror(errno));
		irmo_socket_unref(sock);
		return NULL;
	}

	// bound successfully

	sock->port = port;
	
	return sock;
}

void irmo_socket_block_set(IrmoSocket **sockets, int nsockets, int timeout)
{
	fd_set set;
	struct timeval tv_timeout;
	int result;
	int max;
	int i;
	
	irmo_return_if_fail(sockets != NULL);
	irmo_return_if_fail(nsockets >= 0);
	
	FD_ZERO(&set);

	for (max=0,i=0; i<nsockets; ++i) {
		FD_SET(sockets[i]->sock, &set);
		if (max < sockets[i]->sock)
			max = sockets[i]->sock;
	}

	if (timeout > 0) {
		tv_timeout.tv_sec = timeout / 1000;
		tv_timeout.tv_usec = timeout % 1000;

		result = select(max+1, &set, NULL, NULL, &tv_timeout);
	} else {
		result = select(max+1, &set, NULL, NULL, NULL);
	}
	
	irmo_return_if_fail(result >= 0);
}

void irmo_socket_block(IrmoSocket *socket, int timeout)
{
	irmo_return_if_fail(socket != NULL);

	irmo_socket_block_set(&socket, 1, timeout);
}

