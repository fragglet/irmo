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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netlib.h"
#include "server.h"

IrmoServer *irmo_server_new(IrmoSocket *sock, char *hostname,
			    IrmoWorld *world, IrmoInterfaceSpec *spec)
{
	IrmoServer *server;

	g_return_val_if_fail(sock != NULL, NULL);
	
	// sanity checks; make sure the hostname is not already taken
	
	if (hostname) {
		if (g_hash_table_lookup(sock->servers, hostname)) {
			irmo_error_report("irmo_server_new",
					  "already a server bound to '%s' on socket %i::%i",
					  hostname, sock->domain, sock->port);
			return NULL;
		}
	} else {
		if (sock->default_server) {
			irmo_error_report("irmo_server_new",
					  "already a default server for socket %i::%i\n",
					  sock->domain, sock->port);
			return NULL;
		}
	}

	// create new server
	
	server = g_new0(IrmoServer, 1);
	server->refcount = 1;
	server->socket = sock;
	server->world = world;
	server->client_spec = spec;
	server->running = TRUE;

	irmo_socket_ref(sock);

	if (spec)
		irmo_interface_spec_ref(spec);
	
	// we can have a server which does not serve a world
	// store this server in the list of servers attached to this
	// world
	
	if (world) {
		irmo_world_ref(world);
		g_ptr_array_add(world->servers, server);
	}
	
	if (hostname) {
		server->hostname = strdup(hostname);
		g_hash_table_insert(sock->servers, hostname, server);
	} else {
		server->hostname = NULL;
		sock->default_server = server;
	}

	server->clients = g_hash_table_new((GHashFunc) irmo_sockaddr_hash,
					   (GCompareFunc) irmo_sockaddr_cmp);
	
	return server;
}

void irmo_server_ref(IrmoServer *server)
{
	g_return_if_fail(server != NULL);
	
	++server->refcount;
}

static gboolean remove_each_client(gpointer key, IrmoClient *client,
				   gpointer user_data)
{
	// remove from the socket list
	
	g_hash_table_remove(client->server->socket->clients, key);

	// destroy

	irmo_client_internal_unref(client);
	
	// remove from server list
	
	return TRUE;
}

// remove ourselves from the socket

static void irmo_server_internal_shutdown(IrmoServer *server)
{
	if (!server->running)
		return;

	// remove clients

	g_hash_table_foreach_remove(server->clients,
				    (GHRFunc) remove_each_client,
				    NULL);
		
	// delink from the server
		
	if (server->hostname) {
		g_hash_table_remove(server->socket->servers,
				    server->hostname);
		free(server->hostname);
	} else {
		server->socket->default_server = NULL;
	}

	// remove from list of attached servers
			
	if (server->world)
		g_ptr_array_remove(server->world->servers, server);

	server->running = FALSE;
			
}

void irmo_server_unref(IrmoServer *server)
{
	g_return_if_fail(server != NULL);
	
	--server->refcount;

	if (server->refcount <= 0) {
		
		irmo_server_internal_shutdown(server);
		g_hash_table_destroy(server->clients);

		// destroy callbacks

		irmo_callbacklist_free(server->connect_callbacks);

		irmo_socket_unref(server->socket);
		
		if (server->client_spec)
			irmo_interface_spec_unref(server->client_spec);

		if (server->world)
			irmo_world_unref(server->world);
		
		free(server);
	}
}

IrmoCallback *irmo_server_watch_connect(IrmoServer *server, 
					IrmoClientCallback func,
					void *user_data)
{
	g_return_if_fail(server != NULL);
	g_return_if_fail(func != NULL);
	
	return irmo_callbacklist_add(&server->connect_callbacks, 
				     func, user_data);
}

static void client_callback_raise_foreach(IrmoCallback *data,
					 IrmoClient *client)
{
	IrmoClientCallback func = (IrmoClientCallback) data->func;

	func(client, data->user_data);
}

void irmo_client_callback_raise(GSList *list, IrmoClient *client)
{
	g_slist_foreach(list,
			(GFunc) client_callback_raise_foreach,
			client);
}

void irmo_server_raise_connect(IrmoServer *server, IrmoClient *client)
{
	irmo_client_callback_raise(server->connect_callbacks, client);
}

struct server_foreach_data {
	IrmoClientCallback func;
	gpointer user_data;	
};

static void server_foreach_foreach(gpointer key,
				   IrmoClient *client,
				   struct server_foreach_data *data)
{
	if (client->state == CLIENT_CONNECTED)
		data->func(client, data->user_data);
}

void irmo_server_foreach_client(IrmoServer *server, IrmoClientCallback callback,
				void *user_data)
{
	struct server_foreach_data foreach_data = {
		callback,
		user_data
	};

	g_return_if_fail(server != NULL);
	g_return_if_fail(callback != NULL);

	g_hash_table_foreach(server->clients, 
			     (GHFunc) server_foreach_foreach,
			     &foreach_data);
}

static void server_shutdown_foreach(IrmoClient *client, gpointer user_data)
{
	irmo_client_disconnect(client);
}

void irmo_server_shutdown(IrmoServer *server)
{
	g_return_if_fail(server != NULL);

	// disconnect all clients

	irmo_server_foreach_client(server, server_shutdown_foreach, NULL);

	// run the socket until all clients are disconnected
	
	while (g_hash_table_size(server->clients)) {
		irmo_socket_run(server->socket);
		irmo_socket_block(server->socket, 100);
	}
	
	irmo_server_internal_shutdown(server);
}

// $Log$
// Revision 1.11  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.10  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.9  2003/09/01 19:28:04  fraggle
// Fix typo
//
// Revision 1.8  2003/09/01 18:59:27  fraggle
// Add a timeout parameter for blocking on sockets. Use block function
// internally.
//
// Revision 1.7  2003/09/01 18:41:55  fraggle
// irmo_server_shutdown
//
// Revision 1.6  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.5  2003/08/30 03:08:00  fraggle
// Use irmo_client_internal_unref instead of unreffing externally. Make
// irmo_client_destroy static now.
//
// Revision 1.4  2003/08/15 15:22:42  fraggle
// Add an iterator function to iterate over clients connected to a server.
//
// Revision 1.3  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
// Revision 1.2  2003/07/22 02:05:39  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.18  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.17  2003/04/25 01:26:19  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.16  2003/03/17 17:33:59  sdh300
// Fix arguments to irmo_client_callback_raise
//
// Revision 1.15  2003/03/17 17:16:48  sdh300
// Generalise to invoking lists of IrmoClientCallback callbacks
//
// Revision 1.14  2003/03/16 00:44:04  sdh300
// Add irmo_callbacklist_free function
//
// Revision 1.13  2003/03/14 18:31:36  sdh300
// Generalise callback functions to irmo_callbacklist type,
// remove redundant client_callback code
//
// Revision 1.12  2003/03/07 14:31:19  sdh300
// Callback functions for watching new client connects
//
// Revision 1.11  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.10  2003/03/06 19:33:50  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.9  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.8  2003/02/18 18:39:54  sdh300
// store list of attached servers in universe
//
// Revision 1.7  2003/02/18 17:41:01  sdh300
// fix reference counting (refcount not initialised properly,
// 	object not freed)
//
// Revision 1.6  2003/02/16 23:41:26  sdh300
// Reference counting for client and server objects
//
// Revision 1.5  2003/02/06 01:17:15  sdh300
// Allow servers that do not serve a universe
//
// Revision 1.4  2002/12/02 21:32:50  sdh300
// reference counting for IrmoSockets
//
// Revision 1.3  2002/11/26 16:27:00  sdh300
// server needs a hash of connected clients as well as the parent socket
//
// Revision 1.2  2002/11/26 15:46:41  sdh300
// Fix compile and possible namespace conflicts with the "socket" function
//
// Revision 1.1  2002/11/26 15:43:05  sdh300
// Initial IrmoServer code
//
