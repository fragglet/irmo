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

#ifndef IRMO_INTERNAL_SOCKET_H
#define IRMO_INTERNAL_SOCKET_H

#include "public/socket.h"

#include <glib.h>

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
	
	// reference count
	
	int refcount;
	
	// domain/port and socket

	IrmoSocketDomain domain;
	int port;
	int sock;

	// attached servers hashed by name

	GHashTable *servers;

	// the default server to use (if the hostname a client is trying to
	// connect to does not exist)

	IrmoServer *default_server;
	
	// connected clients hashed by IP

	GHashTable *clients;
};

IrmoSocket *irmo_socket_new_unbound(IrmoSocketDomain domain);
void irmo_socket_sendpacket(IrmoSocket *sock, struct sockaddr *dest,
			    IrmoPacket *packet);

#endif /* #ifndef IRMO_INTERNAL_SOCKET_H */

// $Log$
// Revision 1.3  2003/11/18 18:14:47  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.2  2003/08/26 14:57:31  fraggle
// Remove AF_* BSD sockets dependency from Irmo API
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.10  2003/06/09 21:06:53  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.9  2003/03/07 12:17:18  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.8  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.7  2003/02/23 00:00:04  sdh300
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
