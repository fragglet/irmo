#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netlib.h"
#include "server.h"

IrmoServer *irmo_server_new(IrmoSocket *sock, gchar *hostname,
			    IrmoUniverse *universe, IrmoInterfaceSpec *spec)
{
	IrmoServer *server;
	
	// sanity checks; make sure the hostname is not already taken
	
	if (hostname) {
		if (g_hash_table_lookup(sock->servers, hostname)) {
			fprintf(stderr,
				"irmo_server_new: already a server bound "
				"to '%s' on socket %i::%i\n",
				hostname, sock->domain, sock->port);
			return NULL;
		}
	} else {
		if (sock->default_server) {
			fprintf(stderr,
				"irmo_server_new: already a default server "
				"for socket %i::%i\n",
				sock->domain, sock->port);
			return NULL;
		}
	}

	// create new server
	
	server = g_new0(IrmoServer, 1);
	server->refcount = 1;
	server->socket = sock;
	server->universe = universe;
	server->client_spec = spec;

	irmo_socket_ref(sock);

	if (spec)
		irmo_interface_spec_ref(spec);
	
	// we can have a server which does not serve a universe
	// store this server in the list of servers attached to this
	// universe
	
	if (universe) {
		irmo_universe_ref(universe);
		g_ptr_array_add(universe->servers, server);
	}
	
	if (hostname) {
		server->hostname = strdup(hostname);
		g_hash_table_insert(sock->servers, hostname, server);
	} else {
		server->hostname = NULL;
		sock->default_server = server;
	}

	server->clients = g_hash_table_new((GHashFunc) sockaddr_hash,
					   (GCompareFunc) sockaddr_cmp);
	
	return server;
}

void irmo_server_ref(IrmoServer *server)
{
	++server->refcount;
}

static gboolean server_unref_client_foreach(gpointer key, IrmoClient *client,
					    gpointer user_data)
{
	// remove from the socket list
	
	g_hash_table_remove(client->server->socket->clients, key);

	// destroy

	irmo_client_destroy(client);
	
	// remove from server list
	
	return TRUE;
}

void irmo_server_unref(IrmoServer *server)
{
	--server->refcount;

	if (server->refcount <= 0) {

		// destroy callbacks

		irmo_callbacklist_free(server->connect_callbacks);

		// remove clients

		g_hash_table_foreach_remove(server->clients,
					    (GHRFunc)
					      server_unref_client_foreach,
					    NULL);
		g_hash_table_destroy(server->clients);
		
		// delink from the server
		
		if (server->hostname) {
			g_hash_table_remove(server->socket->servers,
					    server->hostname);
			free(server->hostname);
		} else {
			server->socket->default_server = NULL;
		}

		irmo_socket_unref(server->socket);
		
		if (server->client_spec)
			irmo_interface_spec_unref(server->client_spec);
		if (server->universe) {
			// remove from list of attached servers
			
			g_ptr_array_remove(server->universe->servers,
					   server);
			
			irmo_universe_unref(server->universe);
		}
		
		free(server);
	}
}

void irmo_server_watch_connect(IrmoServer *server, IrmoClientCallback func,
			       gpointer user_data)
{
	irmo_callbacklist_add(&server->connect_callbacks, func, user_data);
}

void irmo_server_unwatch_connect(IrmoServer *server, IrmoClientCallback func,
				 gpointer user_data)
{

	if (!irmo_callbacklist_remove(&server->connect_callbacks, 
				      func, user_data)) {
		fprintf(stderr,
			"irmo_server_unwatch_connect: "
			"watch not found on server\n");
	}
}

static void client_callback_raise_foreach(IrmoCallbackFuncData *data,
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

// $Log: not supported by cvs2svn $
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
