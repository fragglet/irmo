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

#include "sysheaders.h"

#include "error.h"
#include "netlib.h"
#include "server.h"

// create a new server using an existing IrmoSocket object
// used when making client connections

IrmoServer *irmo_server_new_from(IrmoSocket *sock,
				 IrmoWorld *world, 
				 IrmoInterfaceSpec *client_spec)
{
	IrmoServer *server;

	// create new server
	
	server = irmo_new0(IrmoServer, 1);
	server->refcount = 1;
	server->socket = sock;
	server->world = world;
	server->client_spec = client_spec;
	server->running = 1;

	irmo_socket_ref(sock);

	if (client_spec)
		irmo_interface_spec_ref(client_spec);
	
	// we can have a server which does not serve a world
	// store this server in the list of servers attached to this
	// world
	
	if (world) {
		irmo_world_ref(world);
		irmo_arraylist_append(world->servers, server);
	}
	
	// attach ourselves to the server

	sock->server = server;

	server->clients = irmo_hash_table_new((IrmoHashTableHashFunc) irmo_sockaddr_hash,
					      (IrmoHashTableEqualFunc) irmo_sockaddr_cmp);

	return server;
}

IrmoServer *irmo_server_new(IrmoSocketDomain domain, int port,
                            IrmoWorld *world, 
			    IrmoInterfaceSpec *client_spec)
{
	IrmoSocket *sock;
	IrmoServer *server;

	sock = irmo_socket_new_bound(domain, port);

	if (sock == NULL) 
		return NULL;
	
	server = irmo_server_new_from(sock, world, client_spec);

	irmo_socket_unref(sock);
	
	return server;
}

void irmo_server_ref(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);
	
	++server->refcount;
}

static int remove_each_client(void *key, IrmoClient *client,
				   void *user_data)
{
	// destroy

	irmo_client_internal_unref(client);
	
	// remove from server list
	
	return 1;
}

// remove ourselves from the socket

static void irmo_server_internal_shutdown(IrmoServer *server)
{
        int i;

	//printf("shutdown server\n");
	if (!server->running)
		return;

	// remove clients

	irmo_hash_table_foreach_remove(server->clients,
				    (IrmoHashTableRemoveIterator) remove_each_client,
				    NULL);
		
	// shutdown the socket we're using

	irmo_socket_shutdown(server->socket);

	// remove from list of attached servers
			
	if (server->world) {
                for (i=0; i<server->world->servers->length; ++i) {
                        if (server->world->servers->data[i] == server) {
                                irmo_arraylist_remove(server->world->servers, i);
                        }
                }
        }

	server->running = 0;
}

void irmo_server_unref(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);
	
	--server->refcount;

	if (server->refcount <= 0) {
		
		irmo_server_internal_shutdown(server);
		irmo_hash_table_free(server->clients);

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
	irmo_return_val_if_fail(server != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	return irmo_callbacklist_add(&server->connect_callbacks, 
				     func, user_data);
}

static void client_callback_raise_foreach(IrmoCallback *data,
					 IrmoClient *client)
{
	IrmoClientCallback func = (IrmoClientCallback) data->func;

	func(client, data->user_data);
}

void irmo_client_callback_raise(IrmoSListEntry *list, IrmoClient *client)
{
	irmo_slist_foreach(list,
			(IrmoSListIterator) client_callback_raise_foreach,
			client);
}

void irmo_server_raise_connect(IrmoServer *server, IrmoClient *client)
{
	irmo_client_callback_raise(server->connect_callbacks, client);
}

struct server_foreach_data {
	IrmoClientCallback func;
	void *user_data;	
};

static void server_foreach_foreach(void *key,
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

	irmo_return_if_fail(server != NULL);
	irmo_return_if_fail(callback != NULL);

	irmo_hash_table_foreach(server->clients, 
			     (IrmoHashTableIterator) server_foreach_foreach,
			     &foreach_data);
}

static void server_shutdown_foreach(IrmoClient *client, void *user_data)
{
	irmo_client_disconnect(client);
}

void irmo_server_shutdown(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);

	// disconnect all clients

	irmo_server_foreach_client(server, server_shutdown_foreach, NULL);

	// run the socket until all clients are disconnected
	
	while (irmo_hash_table_num_entries(server->clients)) {
		irmo_socket_run(server->socket);
		irmo_socket_block(server->socket, 100);
	}
	
	irmo_server_internal_shutdown(server);
}

IrmoSocket *irmo_server_get_socket(IrmoServer *server)
{
	irmo_return_val_if_fail(server != NULL, NULL);

	return server->socket;
}

void irmo_server_run(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);

	irmo_socket_run(server->socket);
}

