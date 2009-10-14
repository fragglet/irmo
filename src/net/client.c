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

#include <irmo/packet.h>

#include "client.h"
#include "client_sendq.h"
#include "connection.h"
#include "protocol.h"
#include "sendatom.h"

// number of seconds between sending SYN (for client) and SYN ACK (for server)
// on connect

#define CLIENT_CONNECT_ATTEMPTS 6
#define CLIENT_SYN_INTERVAL 1000

// create a new client (used internally)

IrmoClient *irmo_client_new(IrmoServer *server, IrmoNetAddress *addr)
{
	IrmoClient *client;

	client = irmo_new0(IrmoClient, 1);

	client->state = IRMO_CLIENT_CONNECTING;
	client->server = server;
	client->connect_time = 0;
	client->connect_attempts = CLIENT_CONNECT_ATTEMPTS;

        // Set address and add reference

	client->address = addr;
        irmo_net_address_ref(addr);

	// send queue

	client->sendq = irmo_queue_new();
        irmo_alloc_assert(client->sendq != NULL);

        client->sendq_hashtable = irmo_hash_table_new(irmo_pointer_hash,
                                                      irmo_pointer_equal);

        irmo_alloc_assert(client->sendq_hashtable != NULL);

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

	client->cwnd = IRMO_PROTOCOL_MTU;
	client->ssthresh = 65535;

        // assign a new ID for this client:

        client->id = irmo_server_assign_id(server);

	// insert into server hashtable

	irmo_alloc_assert(irmo_hash_table_insert(server->clients,
                                                 client->address,
                                                 client));
        irmo_alloc_assert(irmo_hash_table_insert(server->clients_by_id,
                                                 IRMO_POINTER_KEY(client->id),
                                                 client));

        // We have not yet synced the server's world to the client,
        // unless we are not sharing a world to the client, in which
        // case we are already "synced".

        client->remote_synced = client->server->world == NULL;

        // Same, but the other way round.  The client has not yet
        // synced its world to us, unless it is not sharing a world.

        client->local_synced = client->world == NULL;

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
	unsigned int i;

	// destroy callbacks

        for (i=0; i<IRMO_CLIENT_NUM_STATES; ++i) {
                irmo_callback_list_free(&client->state_change_callbacks[i]);
        }

	// clear send queue

	while (!irmo_queue_is_empty(client->sendq)) {
		IrmoSendAtom *atom;

		atom = (IrmoSendAtom *) irmo_queue_pop_head(client->sendq);

		irmo_sendatom_free(atom);
	}

	irmo_queue_free(client->sendq);

	irmo_hash_table_free(client->sendq_hashtable);

	// destroy sendwindow and all data in it

	for (i=0; i<client->sendwindow_size; ++i) {
		irmo_sendatom_free(client->sendwindow[i]);
        }

	//free(client->sendwindow);

	// destroy receive window and all data in it
	
	for (i=0; i<client->recvwindow_size; ++i) {
		if (client->recvwindow[i] != NULL) {
			irmo_sendatom_free(client->recvwindow[i]);
                }
        }

	free(client->recvwindow);

	if (client->world != NULL) {
		irmo_world_unref(client->world);
        }

	if (client->connection_error != NULL) {
		free(client->connection_error);
        }
	
        irmo_net_address_unref(client->address);
	free(client);
}

void irmo_client_internal_unref(IrmoClient *client)
{
	--client->refcount;

	if (client->refcount <= 0) {
		irmo_client_destroy(client);
        }
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
	unsigned int nowtime;

        nowtime = irmo_get_time();

	if (nowtime >= client->connect_time + CLIENT_SYN_INTERVAL) {

		// run out of connection attempts?

		if (client->connect_attempts <= 0) {
			irmo_connection_error(client, "attempt to connect timed out");
                        irmo_client_set_state(client, IRMO_CLIENT_DISCONNECTED);
			return;
		}
		
		// build and send a new packet

		if (client->server->internal_server) {
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
		
		irmo_net_socket_send_packet(client->server->socket,
				            client->address,
				            packet);

		irmo_packet_free(packet);		
		
		client->connect_time = nowtime;

		--client->connect_attempts;		
	}
}

static void client_run_disconnecting(IrmoClient *client)
{
	IrmoPacket *packet;
	unsigned int nowtime;

        nowtime = irmo_get_time();

	if (nowtime >= client->connect_time + CLIENT_SYN_INTERVAL) {

		// after several attempts, give up and just set them
		// as disconnected
		
		if (client->connect_attempts <= 0) {
                        irmo_client_set_state(client, IRMO_CLIENT_DISCONNECTED);
			return;
		}

		// build a syn fin

		packet = irmo_packet_new();

		irmo_packet_writei16(packet, PACKET_FLAG_SYN|PACKET_FLAG_FIN);

		irmo_net_socket_send_packet(client->server->socket,
				            client->address,
				            packet);

		irmo_packet_free(packet);

		// save the time

		--client->connect_attempts;
		client->connect_time = nowtime;
	}
}

void irmo_client_run(IrmoClient *client)
{
	switch (client->state) {
	case IRMO_CLIENT_DISCONNECTED:
		return;
	case IRMO_CLIENT_CONNECTING:
		client_run_connecting(client);
		break;
	case IRMO_CLIENT_CONNECTED:
        case IRMO_CLIENT_SYNCHRONIZED:
		irmo_proto_run_client(client);
		break;
	case IRMO_CLIENT_DISCONNECTING:
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
	
        irmo_client_set_state(client, IRMO_CLIENT_DISCONNECTING);

	// try to send 6 disconnect attempts before
	// giving up
	
	client->connect_time = 0;
	client->connect_attempts = CLIENT_CONNECT_ATTEMPTS;
}

IrmoCallback *irmo_client_watch_state(IrmoClient *client,
                                      IrmoClientState state,
                                      IrmoClientCallback func,
                                      void *user_data)
{
        IrmoCallbackList *list;

	irmo_return_val_if_fail(client != NULL, NULL);
        irmo_return_val_if_fail(state >= 0, NULL);
        irmo_return_val_if_fail(state < IRMO_CLIENT_NUM_STATES, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);

        list = &client->state_change_callbacks[state];

	return irmo_callback_list_add(list, func, user_data);
}

unsigned int irmo_client_timeout_time(IrmoClient *client)
{
	return (unsigned int) (client->rtt + client->rtt_deviation * 2 + 1);
}

int irmo_client_ping_time(IrmoClient *client)
{
	irmo_return_val_if_fail(client != NULL, -1);
	
	return (int) client->rtt;
}

void irmo_client_set_max_sendwindow(IrmoClient *client, unsigned int max)
{
	irmo_return_if_fail(client != NULL);

	client->local_sendwindow_max = max;

	// send to remote machine
	
	irmo_client_sendq_add_sendwindow(client, max);
}

void irmo_client_get_address(IrmoClient *client, char *buffer,
                             unsigned int buffer_len)
{
        irmo_return_if_fail(client != NULL);
        irmo_return_if_fail(buffer != NULL);

        irmo_net_address_to_string(client->address, buffer, buffer_len);
}

unsigned int irmo_client_get_port(IrmoClient *client)
{
        irmo_return_val_if_fail(client != NULL, 0);

        return irmo_net_address_get_port(client->address);
}

IrmoClientID irmo_client_get_id(IrmoClient *client)
{
        irmo_return_val_if_fail(client != NULL, 0);

        return client->id;
}

IrmoClientState irmo_client_get_state(IrmoClient *client)
{
        irmo_return_val_if_fail(client != NULL, IRMO_CLIENT_DISCONNECTED);

        return client->state;
}

void irmo_client_set_state(IrmoClient *client, IrmoClientState state)
{
        IrmoCallbackList *callback_list;

        if (client->state != state) {
                client->state = state;

                callback_list = &client->state_change_callbacks[state];

                irmo_client_callback_raise(callback_list, client);
        }
}

