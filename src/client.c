#include <stdlib.h>
#include <time.h>

#include "client.h"
#include "netlib.h"
#include "packet.h"

// number of seconds between sending SYN (for client) and SYN ACK (for server)
// on connect

#define CLIENT_CONNECT_ATTEMPTS 6
#define CLIENT_SYN_INTERVAL 1

// create a new client (used internally)

IrmoClient * _client_new(IrmoServer *server, struct sockaddr *addr)
{
	IrmoClient *client;

	client = g_new0(IrmoClient, 1);

	client->state = CLIENT_CONNECTING;
	client->server = server;
	client->addr = sockaddr_copy(addr);
	client->_connect_time = 0;
	client->_connect_attempts = CLIENT_CONNECT_ATTEMPTS;

	// note on refcounts for clients:
	// reference counting is different for client objects.
	// in the server, when a client is disconnected it goes into
	// the CLIENT_DISCONNECTED state, but is kept in the server's
	// clients list. the server removes disconnected clients
	// in its pass through the list in socket_run. however, it
	// only removes clients if their refcount == 0. this allows
	// 'hooks' to be kept on particular clients the user may be
	// watching.
	
	client->refcount = 0;
	
	// insert into server hashtable and socket hashtable
	g_hash_table_insert(server->clients,
			    client->addr,
			    client);
	g_hash_table_insert(server->socket->clients,
			    client->addr,
			    client);
	
	return client;
}

void client_ref(IrmoClient *client)
{
	// when you reference a client, you're effectively referencing
	// the server its part of

	server_ref(client->server);
	
	++client->refcount;
}

void client_unref(IrmoClient *client)
{
	--client->refcount;
	
	server_unref(client->server);	
}

void _client_destroy(IrmoClient *client)
{
	free(client->addr);
	free(client);
}

// run when in the connecting state

static void client_run_connecting(IrmoClient *client)
{
	IrmoPacket *packet;
	time_t nowtime = time(NULL);

	if (nowtime >= client->_connect_time + CLIENT_SYN_INTERVAL) {
		// build and send a new packet

		if (client->server->socket->type == SOCKET_CLIENT) {
			IrmoUniverse *local_universe
				= client->server->universe;
			InterfaceSpec *spec = client->server->client_spec;

			packet = packet_new(128); // static: fixme

			// this is the client making a connection to
			// the server

			packet_writei16(packet, PACKET_FLAG_SYN);
			packet_writei32(packet,
					local_universe ?
					local_universe->spec->hash : 0);
			packet_writei32(packet, spec ? spec->hash : 0);

			// no hostname yet, fixme
		} else {
			// we are the server, sending syn ack replies
			// to the connecting client

			packet = packet_new(2);

			packet_writei16(packet,
					PACKET_FLAG_SYN|PACKET_FLAG_ACK);
		}
		
		_socket_sendpacket(client->server->socket,
				   client->addr,
				   packet);

		packet_free(packet);		
		
		client->_connect_time = nowtime;

		--client->_connect_attempts;
		
	}
}

// called by socket_run for each client connected

void _client_run(IrmoClient *client)
{
	switch (client->state) {
	case CLIENT_DISCONNECTED:
		return;
	case CLIENT_CONNECTING:
		client_run_connecting(client);
		break;
	case CLIENT_CONNECTED:
		break;
	}
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.1  2003/02/03 20:57:22  sdh300
// Initial client code
//
