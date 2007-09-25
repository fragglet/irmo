// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
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
//---------------------------------------------------------------------

#ifndef IRMO_INTERNAL_SOCKET_H
#define IRMO_INTERNAL_SOCKET_H

#include "public/socket.h"

#include "netlib.h"
#include "client.h"
#include "server.h"

// fixme: packet.h is supposed to be internal and should not be here
// split off internal socket stuff to a seperate header
#include "packet.h"

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

	// server using this socket
	
	IrmoServer *server;
};

IrmoSocket *irmo_socket_new_bound(IrmoSocketDomain domain, int port);
IrmoSocket *irmo_socket_new_unbound(IrmoSocketDomain domain);
void irmo_socket_sendpacket(IrmoSocket *sock, struct sockaddr *dest,
			    IrmoPacket *packet);
void irmo_socket_shutdown(IrmoSocket *sock);

#endif /* #ifndef IRMO_INTERNAL_SOCKET_H */

