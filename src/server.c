#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	server->socket = sock;
	server->universe = universe;
	server->client_spec = spec;

	if (hostname) {
		server->hostname = strdup(hostname);
		g_hash_table_insert(sock->servers, hostname, server);
	} else {
		server->hostname = NULL;
		sock->default_server = server;
	}

	// TODO: add hooks for universe, store a list of servers connected
	// to each universe
	
	return server;
}


// $Log: not supported by cvs2svn $
// Revision 1.1  2002/11/26 15:43:05  sdh300
// Initial IrmoServer code
//