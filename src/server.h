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

#ifndef IRMO_INTERNAL_SERVER_H
#define IRMO_INTERNAL_SERVER_H

#include "public/server.h"

#include "if_spec.h"
#include "client.h"
#include "socket.h"
#include "world.h"

struct _IrmoServer {

	// reference count
	
	int refcount;
	
	// vhost hostname

	gchar *hostname;

	// socket this server is using

	IrmoSocket *socket;
	
	// world being served by this server
	
	IrmoWorld *world;

	// spec for client worlds

	IrmoInterfaceSpec *client_spec;
	
	// connected IrmoClients hashed by IP
	
	GHashTable *clients;

	// callback functions for new connections

	GSList *connect_callbacks;
};

// invoke IrmoClientCallback callback functions in a list

void irmo_client_callback_raise(GSList *list, IrmoClient *client);

// raise callback functions on new client connect

void irmo_server_raise_connect(IrmoServer *server, IrmoClient *client);

#endif /* #ifndef IRMO_INTERNAL_SERVER_H */

// $Log$
// Revision 1.2  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.10  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.9  2003/03/17 17:33:59  sdh300
// Fix arguments to irmo_client_callback_raise
//
// Revision 1.8  2003/03/17 17:16:48  sdh300
// Generalise to invoking lists of IrmoClientCallback callbacks
//
// Revision 1.7  2003/03/07 14:31:19  sdh300
// Callback functions for watching new client connects
//
// Revision 1.6  2003/03/06 19:33:51  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.5  2003/02/23 00:00:04  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.4  2003/02/16 23:41:27  sdh300
// Reference counting for client and server objects
//
// Revision 1.3  2002/11/26 15:46:41  sdh300
// Fix compile and possible namespace conflicts with the "socket" function
//
// Revision 1.2  2002/11/26 15:43:05  sdh300
// Initial IrmoServer code
//
// Revision 1.1  2002/11/26 15:23:49  sdh300
// Initial network code
//

