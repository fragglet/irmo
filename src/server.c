#include "server.h"


IrmoServer *server_new(IrmoSocket *socket, gchar *hostname,
		       IrmoUniverse *universe, InterfaceSpec *spec)
{
	IrmoServer *server;
	
	// sanity checks; make sure the hostname is not already taken
	
	if (hostname) {
		if (g_hash_table_lookup(socket->servers, hostname)) {
			fprintf(stderr,
				"server_new: already a server bound to '%s' "
				"on socket %i::%i\n",
				hostname, socket->domain, socket->port);
			return NULL;
		}
	} else {
		if (socket->default_server) {
			fprintf(stderr,
				"server_new: already a default server for "
				"socket %i::%i\n",
				socket->domain, socket->port);
			return NULL;
		}
	}

	// create new server
	
	server = g_new0(IrmoServer, 1);
	server->socket = socket;
	server->universe = universe;
	server->client_spec = spec;

	if (hostname) {
		server->hostname = strdup(hostname);
		g_hash_table_insert(socket->servers, hostname, server);
	} else {
		server->hostname = NULL;
		socket->default_server = server;
	}

	// TODO: add hooks for universe, store a list of servers connected
	// to each universe
	
	return server;
}


// $Log: not supported by cvs2svn $
