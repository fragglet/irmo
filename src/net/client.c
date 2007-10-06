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

#include "netbase/netlib.h"
#include <irmo/packet.h>

#include "client.h"
#include "connection.h"
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

	client = irmo_new0(IrmoClient, 1);

	client->state = CLIENT_CONNECTING;
	client->server = server;
	client->addr = irmo_sockaddr_copy(addr);
	client->connect_time = 0;
	client->connect_attempts = CLIENT_CONNECT_ATTEMPTS;

	// send queue

	client->sendq = irmo_queue_new();
	client->sendq_hashtable = irmo_hash_table_new(irmo_pointer_hash,
						   irmo_pointer_equal);
	
	// receive window

	client->recvwindow_start = 0;
	client->recvwindow_size = 64;
	client->recvwindow = irmo_new0(IrmoSendAtom *, client->recvwindow_size);

	// start at one ref, from the server this is part of 

	client->refcount = 1;

	// initial rtt mean/stddev

	client->rtt = 1000;
	client->rtt_deviation = 200;

	// backoff

	client->backoff = 1;

	// congestion/sendwindow size stuff

	// start cwnd with one packet, ssthresh as large

	client->cwnd = PACKET_THRESHOLD;
	client->ssthresh = 65535;

	// insert into server hashtable and socket hashtable
	
	irmo_hash_table_insert(server->clients,
			    client->addr,
			    client);
	
	return client;
}

void irmo_client_ref(IrmoClient *client)
{
	irmo_return_if_fail(client != NULL);
	
	// when you reference a client, you're effectively referencing
	// the server its part of

	irmo_server_ref(client->server);
	
	++client->refcount;
}

static void irmo_client_destroy(IrmoClient *client)
{
	int i;

	// destroy callbacks
	
	irmo_callback_list_free(&client->disconnect_callbacks);

	// clear send queue

	while (!irmo_queue_is_empty(client->sendq)) {
		IrmoSendAtom *atom;

		atom = (IrmoSendAtom *) irmo_queue_pop_head(client->sendq);

		irmo_sendatom_free(atom);
	}

	irmo_queue_free(client->sendq);

	irmo_hash_table_free(client->sendq_hashtable);

	// destroy sendwindow and all data in it
	
	for (i=0; i<client->sendwindow_size; ++i)
		irmo_sendatom_free(client->sendwindow[i]);

	//free(client->sendwindow);

	// destroy receive window and all data in it
	
	for (i=0; i<client->recvwindow_size; ++i)
		if (client->recvwindow[i])
			irmo_sendatom_free(client->recvwindow[i]);

	free(client->recvwindow);

	if (client->world)
		irmo_world_unref(client->world);

	if (client->connection_error)
		free(client->connection_error);
	
	free(client->addr);
	free(client);
}

void irmo_client_internal_unref(IrmoClient *client)
{
	--client->refcount;

	if (client->refcount <= 0)
		irmo_client_destroy(client);
}

void irmo_client_unref(IrmoClient *client)
{
	irmo_return_if_fail(client != NULL);

	irmo_client_internal_unref(client);
	
	irmo_server_unref(client->server);	
}

// run when in the connecting state

static void client_run_connecting(IrmoClient *client)
{
	IrmoPacket *packet;
	time_t nowtime = time(NULL);

	if (nowtime >= client->connect_time + CLIENT_SYN_INTERVAL) {

		// run out of connection attempts?

		if (client->connect_attempts <= 0) {
			client->state = CLIENT_DISCONNECTED;
			irmo_connection_error(client, "attempt to connect timed out");
			return;
		}
		
		// build and send a new packet

		if (client->server->socket->type == SOCKET_CLIENT) {
			IrmoWorld *local_world
				= client->server->world;
			IrmoInterface *iface = client->server->client_interface;
                        unsigned int local_hash, remote_hash;

			packet = irmo_packet_new(); 

			// this is the client making a connection to
			// the server

                        if (local_world != NULL) {
                                local_hash = irmo_interface_hash(local_world->iface);
                        } else {
                                local_hash = 0;

                        }
                        if (iface != NULL) {
                                remote_hash = irmo_interface_hash(iface);
                        } else {
                                remote_hash = 0;
                        }


			irmo_packet_writei16(packet, PACKET_FLAG_SYN);
			irmo_packet_writei16(packet, IRMO_PROTOCOL_VERSION);
			irmo_packet_writei32(packet, local_hash);
			irmo_packet_writei32(packet, remote_hash);

			// no hostname yet, fixme
		} else {
			// we are the server, sending syn ack replies
			// to the connecting client

			packet = irmo_packet_new();

			irmo_packet_writei16(packet,
					     PACKET_FLAG_SYN|PACKET_FLAG_ACK);
		}
		
		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		irmo_packet_free(packet);		
		
		client->connect_time = nowtime;

		--client->connect_attempts;		
	}
}

static void client_run_disconnecting(IrmoClient *client)
{
	IrmoPacket *packet;
	time_t nowtime = time(NULL);

	if (nowtime >= client->connect_time + CLIENT_SYN_INTERVAL) {

		// after several attempts, give up and just set them
		// as disconnected
		
		if (client->connect_attempts <= 0) {
			client->state = CLIENT_DISCONNECTED;
			return;
		}

		// build a syn fin

		packet = irmo_packet_new();

		irmo_packet_writei16(packet, PACKET_FLAG_SYN|PACKET_FLAG_FIN);

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		irmo_packet_free(packet);

		// save the time

		--client->connect_attempts;
		client->connect_time = nowtime;
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
		irmo_proto_run_client(client);
		break;
	case CLIENT_DISCONNECTING:
		client_run_disconnecting(client);
		break;
        default:
                irmo_bug();
	}
}

IrmoWorld *irmo_client_get_world(IrmoClient *client)
{
	irmo_return_val_if_fail(client != NULL, NULL);
	
	return client->world;
}

void irmo_client_disconnect(IrmoClient *client)
{
	// set into the disconnecting state
	
	client->state = CLIENT_DISCONNECTING;

	// try to send 6 disconnect attempts before
	// giving up
	
	client->connect_time = 0;
	client->connect_attempts = CLIENT_CONNECT_ATTEMPTS;
}

IrmoCallback *irmo_client_watch_disconnect(IrmoClient *client,
					   IrmoClientCallback func,
					   void *user_data)
{
	irmo_return_val_if_fail(client != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	return irmo_callback_list_add(&client->disconnect_callbacks,
                                      func, user_data);
}

int irmo_client_timeout_time(IrmoClient *client)
{
	return (int) (client->rtt + client->rtt_deviation * 2 + 1);
}

int irmo_client_ping_time(IrmoClient *client)
{
	irmo_return_val_if_fail(client != NULL, -1);
	
	return (int) client->rtt;
}

void irmo_client_set_max_sendwindow(IrmoClient *client, int max)
{
	irmo_return_if_fail(client != NULL);

	client->local_sendwindow_max = max;

	// send to remote machine
	
	irmo_client_sendq_add_sendwindow(client, max);
}

const char *irmo_client_get_addr(IrmoClient *client)
{
	static char buf[128];

	irmo_return_val_if_fail(client != NULL, NULL);

	switch (client->addr->sa_family) {
	case AF_INET: {
		struct sockaddr_in *addr = (struct sockaddr_in *) client->addr;
		return inet_ntoa(addr->sin_addr);
	}
#ifdef USE_IPV6
	case AF_INET6: {
		return inet_ntop(AF_INET6, client->addr, buf, sizeof(buf)-1);
	}
#endif
	}

	return NULL;
}

