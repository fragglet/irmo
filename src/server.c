#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "netlib.h"
#include "server.h"

IrmoServer *server_new(IrmoSocket *sock, gchar *hostname,
		       IrmoUniverse *universe, InterfaceSpec *spec)
{
	IrmoServer *server;
	
	// sanity checks; make sure the hostname is not already taken
	
	if (hostname) {
		if (g_hash_table_lookup(sock->servers, hostname)) {
			fprintf(stderr,
				"server_new: already a server bound to '%s' "
				"on socket %i::%i\n",
				hostname, sock->domain, sock->port);
			return NULL;
		}
	} else {
		if (sock->default_server) {
			fprintf(stderr,
				"server_new: already a default server for "
				"socket %i::%i\n",
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

	socket_ref(sock);

	if (spec)
		interface_spec_ref(spec);
	
	// we can have a server which does not serve a universe
	// store this server in the list of servers attached to this
	// universe
	
	if (universe) {
		universe_ref(universe);
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

void server_ref(IrmoServer *server)
{
	++server->refcount;
}

static gboolean server_unref_client_foreach(gpointer key, IrmoClient *client,
					    gpointer user_data)
{
	// remove from the socket list
	
	g_hash_table_remove(client->server->socket->clients, key);

	// destroy

	client_destroy(client);
	
	// remove from server list
	
	return TRUE;
}

void server_unref(IrmoServer *server)
{
	--server->refcount;

	if (server->refcount <= 0) {

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

		socket_unref(server->socket);
		
		if (server->client_spec)
			interface_spec_unref(server->client_spec);
		if (server->universe) {
			// remove from list of attached servers
			
			g_ptr_array_remove(server->universe->servers,
					   server);
			
			universe_unref(server->universe);
		}
		
		free(server);
	}
}

// $Log: not supported by cvs2svn $
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
