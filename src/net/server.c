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

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"
#include "base/iterator.h"

#include "netbase/netlib.h"

#include "server.h"

// create a new server using an existing IrmoSocket object
// used when making client connections

IrmoServer *irmo_server_new_from(IrmoSocket *sock,
				 IrmoWorld *world, 
				 IrmoInterface *client_interface)
{
	IrmoServer *server;

	// create new server
	
	server = irmo_new0(IrmoServer, 1);
	server->refcount = 1;
	server->socket = sock;
	server->world = world;
	server->client_interface = client_interface;
	server->running = 1;

	irmo_socket_ref(sock);

	if (client_interface)
		irmo_interface_ref(client_interface);
	
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
			    IrmoInterface *client_interface)
{
	IrmoSocket *sock;
	IrmoServer *server;

	sock = irmo_socket_new_bound(domain, port);

	if (sock == NULL) 
		return NULL;
	
	server = irmo_server_new_from(sock, world, client_interface);

	irmo_socket_unref(sock);
	
	return server;
}

void irmo_server_ref(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);
	
	++server->refcount;
}

static void irmo_server_remove_all_clients(IrmoServer *server)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {
                client = irmo_hash_table_iter_next(iter);

                // Remove this client from the hash table and destroy it.

                irmo_hash_table_remove(server->clients, client->addr);

                irmo_client_internal_unref(client);
        }

        irmo_hash_table_iter_free(iter);
}

// remove ourselves from the socket

static void irmo_server_internal_shutdown(IrmoServer *server)
{
        int i;

	//printf("shutdown server\n");
	if (!server->running)
		return;

	// remove clients

        irmo_server_remove_all_clients(server);

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

		irmo_callbacklist_free(&server->connect_callbacks);

		irmo_socket_unref(server->socket);
		
		if (server->client_interface)
			irmo_interface_unref(server->client_interface);

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

void irmo_client_callback_raise(IrmoSListEntry **list, IrmoClient *client)
{
        IrmoSListIterator *iter;
        IrmoClientCallback func;
        IrmoCallback *callback;

        // Invoke all callbacks

        iter = irmo_slist_iterate(list);

        while (irmo_slist_iter_has_more(iter)) {
                callback = irmo_slist_iter_next(iter);

                func = (IrmoClientCallback) callback->func;
                func(client, callback->user_data);
        }

        irmo_slist_iter_free(iter);
}

void irmo_server_raise_connect(IrmoServer *server, IrmoClient *client)
{
	irmo_client_callback_raise(&server->connect_callbacks, client);
}

IrmoIterator *irmo_server_iterate_clients(IrmoServer *server)
{
	irmo_return_val_if_fail(server != NULL, NULL);

        return irmo_iterate_hash_table(server->clients);
}

static void server_disconnect_all_clients(IrmoServer *server)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {

                client = irmo_hash_table_iter_next(iter);

                irmo_client_disconnect(client);
        }

        irmo_hash_table_iter_free(iter);
}

void irmo_server_shutdown(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);

	// disconnect all clients

        server_disconnect_all_clients(server);

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

