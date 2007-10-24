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

#ifndef IRMO_NET_SOCKET_H
#define IRMO_NET_SOCKET_H

#include <irmo/socket.h>

#include "netbase/netlib.h"

#include "client.h"
#include "server.h"

#include <irmo/packet.h>

typedef enum {
	SOCKET_SERVER,
	SOCKET_CLIENT,
} IrmoSocketType;

struct _IrmoSocket {

	IrmoSocketType type;

	// if true, this has been shutdown
	
	int shutdown;
	
	// reference count
	
	int refcount;
	
	// domain/port and socket

	IrmoSocketDomain domain;
	int port;
	int sock;
};

IrmoSocket *irmo_socket_new_bound(IrmoSocketDomain domain, int port);
IrmoSocket *irmo_socket_new_unbound(IrmoSocketDomain domain);
void irmo_socket_sendpacket(IrmoSocket *sock, struct sockaddr *dest,
			    IrmoPacket *packet);
void irmo_socket_shutdown(IrmoSocket *sock);
int irmo_socket_type_to_domain(IrmoSocketDomain type);

#endif /* #ifndef IRMO_NET_SOCKET_H */

