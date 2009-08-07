//
// Copyright (C) 2002-2008 Simon Howard
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
#include "base/alloc.h"
#include "base/assert.h"
#include "base/error.h"
#include "base/iterator.h"

#include "server.h"

// Create a new server using an existing IrmoNetSocket.
// Used when making client connections.

IrmoServer *irmo_server_new_from(IrmoNetSocket *sock,
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

	if (client_interface != NULL) {
		irmo_interface_ref(client_interface);
        }
	
	// we can have a server which does not serve a world
	// store this server in the list of servers attached to this
	// world
	
	if (world != NULL) {
		irmo_world_ref(world);
		irmo_arraylist_append(world->servers, server);
	}
	
	server->clients = irmo_hash_table_new(irmo_pointer_hash,
                                              irmo_pointer_equal);
	server->clients_by_id = irmo_hash_table_new(irmo_pointer_hash,
                                                    irmo_pointer_equal);

	return server;
}

IrmoServer *irmo_server_new(IrmoNetModule *net_module,
                            unsigned int port,
                            IrmoWorld *world, 
			    IrmoInterface *client_interface)
{
	IrmoNetSocket *sock;
	IrmoServer *server;

	sock = irmo_net_socket_open_bound(net_module, port);

	if (sock == NULL) {
		return NULL;
        }
	
	server = irmo_server_new_from(sock, world, client_interface);

	return server;
}

void irmo_server_ref(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);
	
	++server->refcount;
}

// Find an unused client ID for a new client.

IrmoClientID irmo_server_assign_id(IrmoServer *server)
{
        IrmoClientID result;

        // Loop until next_id reaches an unused ID.

        do {
                result = server->next_id;

                server->next_id = (server->next_id + 1) & 0xffff;

        } while (irmo_hash_table_lookup(server->clients_by_id,
                                        IRMO_POINTER_KEY(result)) != NULL);

        return result;
}

static void irmo_server_remove_all_clients(IrmoServer *server)
{
        IrmoHashTableIterator iter;
        IrmoClient *client;

        irmo_hash_table_iterate(server->clients, &iter);

        while (irmo_hash_table_iter_has_more(&iter)) {
                client = irmo_hash_table_iter_next(&iter);

                // Remove this client from the hash table and destroy it.

                irmo_hash_table_remove(server->clients, client->address);
                irmo_hash_table_remove(server->clients_by_id,
                                       IRMO_POINTER_KEY(client->id));

                irmo_client_internal_unref(client);
        }
}

static void irmo_server_internal_shutdown(IrmoServer *server)
{
        unsigned int i;

	//printf("shutdown server\n");
	if (!server->running) {
		return;
        }

	// remove clients

        irmo_server_remove_all_clients(server);

	// shutdown the socket we're using

	irmo_net_socket_close(server->socket);

	// remove from list of attached servers
			
	if (server->world != NULL) {
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
		irmo_hash_table_free(server->clients_by_id);

		// destroy callbacks

		irmo_callback_list_free(&server->connect_callbacks);

		if (server->client_interface != NULL) {
			irmo_interface_unref(server->client_interface);
                }

		if (server->world != NULL) {
			irmo_world_unref(server->world);
		}

		free(server);
	}
}

IrmoCallback *irmo_server_watch_connect(IrmoServer *server, 
					IrmoClientCallback func,
					void *user_data)
{
	irmo_return_val_if_fail(server != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	return irmo_callback_list_add(&server->connect_callbacks, 
				      func, user_data);
}

void irmo_client_callback_raise(IrmoCallbackList *list, IrmoClient *client)
{
        IrmoSListIterator iter;
        IrmoClientCallback func;
        IrmoCallback *callback;

        // Invoke all callbacks

        irmo_slist_iterate(list, &iter);

        while (irmo_slist_iter_has_more(&iter)) {
                callback = irmo_slist_iter_next(&iter);

                func = (IrmoClientCallback) callback->func;
                func(client, callback->user_data);
        }
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
        IrmoHashTableIterator iter;
        IrmoClient *client;

        irmo_hash_table_iterate(server->clients, &iter);

        while (irmo_hash_table_iter_has_more(&iter)) {

                client = irmo_hash_table_iter_next(&iter);

                irmo_client_disconnect(client);
        }
}

void irmo_server_shutdown(IrmoServer *server)
{
	irmo_return_if_fail(server != NULL);

	// disconnect all clients

        server_disconnect_all_clients(server);

	// run the socket until all clients are disconnected
	
	while (irmo_hash_table_num_entries(server->clients)) {
		irmo_server_run(server);
		irmo_net_socket_block(server->socket, 100);
	}
	
	irmo_server_internal_shutdown(server);
}

void irmo_server_block(IrmoServer *server, int ms)
{
        irmo_net_socket_block(server->socket, ms);
}

