#include "client.h"
#include "netlib.h"

IrmoClient * _client_new(IrmoServer *server, struct sockaddr *addr)
{
	IrmoClient *client;

	client = g_new0(IrmoClient, 1);

	client->state = CLIENT_CONNECTING;
	client->server = server;
	client->addr = sockaddr_copy(addr);

	// insert into server hashtable and socket hashtable

	g_hash_table_insert(server->clients,
			    client->addr,
			    client);
	g_hash_table_insert(server->socket->clients,
			    client->addr,
			    client);
	
	return client;
}

// $Log: not supported by cvs2svn $
