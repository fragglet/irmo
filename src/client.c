#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "client.h"
#include "netlib.h"
#include "packet.h"
#include "protocol.h"
#include "sendatom.h"

// number of seconds between sending SYN (for client) and SYN ACK (for server)
// on connect

#define CLIENT_CONNECT_ATTEMPTS 6
#define CLIENT_SYN_INTERVAL 1

// create a new client (used internally)

IrmoClient *irmo_client_new(IrmoServer *server, struct sockaddr *addr)
{
	IrmoClient *client;

	client = g_new0(IrmoClient, 1);

	client->state = CLIENT_CONNECTING;
	client->server = server;
	client->addr = sockaddr_copy(addr);
	client->_connect_time = 0;
	client->_connect_attempts = CLIENT_CONNECT_ATTEMPTS;

	// send queue

	client->sendq = g_queue_new();
	client->sendq_hashtable = g_hash_table_new(g_direct_hash,
						   g_direct_equal);
	
	// receive window

	client->recvwindow_start = 0;
	client->recvwindow_size = 64;
	client->recvwindow = g_new0(IrmoSendAtom *, client->recvwindow_size);
	
	// note on refcounts for clients:
	// reference counting is different for client objects.
	// in the server, when a client is disconnected it goes into
	// the CLIENT_DISCONNECTED state, but is kept in the server's
	// clients list. the server removes disconnected clients
	// in its pass through the list in irmo_socket_run. however, it
	// only removes clients if their refcount == 0. this allows
	// 'hooks' to be kept on particular clients the user may be
	// watching.
	
	client->refcount = 0;

	// initial rtt mean/stddev

	client->rtt = 3000;
	client->rtt_deviation = 1000;

	// backoff

	client->backoff = 1;

	// congestion/sendwindow size stuff

	// start cwnd with one packet, ssthresh as large

	client->cwnd = PACKET_THRESHOLD;
	client->ssthresh = 65535;
	
	// insert into server hashtable and socket hashtable
	
	g_hash_table_insert(server->clients,
			    client->addr,
			    client);
	g_hash_table_insert(server->socket->clients,
			    client->addr,
			    client);
	
	return client;
}

void irmo_client_ref(IrmoClient *client)
{
	// when you reference a client, you're effectively referencing
	// the server its part of

	irmo_server_ref(client->server);
	
	++client->refcount;
}

void irmo_client_unref(IrmoClient *client)
{
	--client->refcount;
	
	irmo_server_unref(client->server);	
}

void irmo_client_destroy(IrmoClient *client)
{
	int i;

	// clear send queue

	while (!g_queue_is_empty(client->sendq)) {
		IrmoSendAtom *atom;

		atom = (IrmoSendAtom *) g_queue_pop_head(client->sendq);

		sendatom_free(atom);
	}

	g_queue_free(client->sendq);

	g_hash_table_destroy(client->sendq_hashtable);
	
	// destroy sendwindow and all data in it
	
	for (i=0; i<client->sendwindow_size; ++i)
		sendatom_free(client->sendwindow[i]);

	//free(client->sendwindow);

	// destroy receive window and all data in it
	
	for (i=0; i<client->recvwindow_size; ++i)
		if (client->recvwindow[i])
			sendatom_free(client->recvwindow[i]);

	free(client->recvwindow);

	if (client->universe)
		irmo_universe_unref(client->universe);
	
	free(client->addr);
	free(client);
}

// run when in the connecting state

static void client_run_connecting(IrmoClient *client)
{
	IrmoPacket *packet;
	time_t nowtime = time(NULL);

	if (nowtime >= client->_connect_time + CLIENT_SYN_INTERVAL) {

		// run out of connection attempts?

		if (client->_connect_attempts <= 0) {
			client->state = CLIENT_DISCONNECTED;
			return;
		}
		
		// build and send a new packet

		if (client->server->socket->type == SOCKET_CLIENT) {
			IrmoUniverse *local_universe
				= client->server->universe;
			IrmoInterfaceSpec *spec = client->server->client_spec;

			packet = packet_new(4); 

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
		
		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		packet_free(packet);		
		
		client->_connect_time = nowtime;

		--client->_connect_attempts;		
	}
}

static void client_run_disconnecting(IrmoClient *client)
{
	IrmoPacket *packet;
	time_t nowtime = time(NULL);

	if (nowtime >= client->_connect_time + CLIENT_SYN_INTERVAL) {

		// after several attempts, give up and just set them
		// as disconnected
		
		if (client->_connect_attempts <= 0) {
			client->state = CLIENT_DISCONNECTED;
			return;
		}

		// build a syn fin

		packet = packet_new(2);

		packet_writei16(packet, PACKET_FLAG_SYN|PACKET_FLAG_FIN);

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		packet_free(packet);

		// save the time

		--client->_connect_attempts;
		client->_connect_time = nowtime;
	}
}

// called by irmo_socket_run for each client connected

void irmo_client_run(IrmoClient *client)
{
	switch (client->state) {
	case CLIENT_DISCONNECTED:
		return;
	case CLIENT_CONNECTING:
		client_run_connecting(client);
		break;
	case CLIENT_CONNECTED:
		proto_run_client(client);
		break;
	case CLIENT_DISCONNECTING:
		client_run_disconnecting(client);
		break;
	}
}

IrmoUniverse *irmo_client_get_universe(IrmoClient *client)
{
	return client->universe;
}

void irmo_client_disconnect(IrmoClient *client)
{
	// set into the disconnecting state
	
	client->state = CLIENT_DISCONNECTING;

	// try to send 6 disconnect attempts before
	// giving up
	
	client->_connect_time = 0;
	client->_connect_attempts = CLIENT_CONNECT_ATTEMPTS;
}

void irmo_client_watch_disconnect(IrmoClient *client,
				  IrmoClientCallback func,
				  gpointer user_data)
{
	irmo_callbacklist_add(&client->disconnect_callbacks,
			      func, user_data);
}

void irmo_client_unwatch_disconnect(IrmoClient *client,
				    IrmoClientCallback func,
				    gpointer user_data)
{
	if (!irmo_callbacklist_remove(&client->disconnect_callbacks,
				      func, user_data)) {
		fprintf(stderr,
			"irmo_client_unwatch_disconnect: "
			"disconnect watch not found!\n");
		return;
	}
}

int irmo_client_timeout_time(IrmoClient *client)
{
	return (int) (client->rtt + client->rtt_deviation * 2 + 1);
}

// $Log: not supported by cvs2svn $
// Revision 1.19  2003/03/21 17:21:45  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.18  2003/03/17 17:34:27  sdh300
// Add disconnect callbacks for clients
//
// Revision 1.17  2003/03/17 17:02:23  sdh300
// Add some missing destroy/unref calls to irmo_client_destroy
//
// Revision 1.16  2003/03/17 16:48:22  sdh300
// Add ability to disconnect from servers and to disconnect clients
//
// Revision 1.15  2003/03/14 17:33:08  sdh300
// Fix crash on trying to free sendwindow
//
// Revision 1.14  2003/03/07 12:31:50  sdh300
// Add protocol.h
//
// Revision 1.13  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.12  2003/03/06 19:33:50  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.11  2003/03/05 17:37:11  sdh300
// Initialise receive window
// Free receive window and send window in destructor
//
// Revision 1.10  2003/03/03 22:14:44  sdh300
// Initial window construction and sending of packets
//
// Revision 1.9  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.8  2003/02/23 00:45:39  sdh300
// Add universe access functions for client, connection
//
// Revision 1.7  2003/02/20 18:24:59  sdh300
// Use GQueue instead of a GPtrArray for the send queue
// Initial change/destroy code
//
// Revision 1.6  2003/02/18 20:04:39  sdh300
// Automatically increase size of packets when writing
//
// Revision 1.5  2003/02/18 18:25:40  sdh300
// Initial queue object code
//
// Revision 1.4  2003/02/18 17:41:37  sdh300
// Add timeout for connect (6 attempts)
//
// Revision 1.3  2003/02/16 23:41:26  sdh300
// Reference counting for client and server objects
//
// Revision 1.2  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.1  2003/02/03 20:57:22  sdh300
// Initial client code
//
