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

#ifndef IRMO_INTERNAL_SERVER_H
#define IRMO_INTERNAL_SERVER_H

#include <irmo/server.h>

#include "interface/interface.h"
#include "world/world.h"

#include "client.h"
#include "socket.h"

struct _IrmoServer {

	// reference count
	
	int refcount;

	// server running?
	
	int running;
	
	// socket this server is using

	IrmoSocket *socket;
	
	// world being served by this server
	
	IrmoWorld *world;

	// interface for client worlds

	IrmoInterface *client_interface;
	
	// connected IrmoClients hashed by IP
	
	IrmoHashTable *clients;

	// callback functions for new connections

	IrmoSListEntry *connect_callbacks;
};

// make a new server using an existing socket object

IrmoServer *irmo_server_new_from(IrmoSocket *sock, IrmoWorld *world,
				 IrmoInterface *client_interface);

// invoke IrmoClientCallback callback functions in a list

void irmo_client_callback_raise(IrmoSListEntry *list, IrmoClient *client);

// raise callback functions on new client connect

void irmo_server_raise_connect(IrmoServer *server, IrmoClient *client);

#endif /* #ifndef IRMO_INTERNAL_SERVER_H */

