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

//
// Base socket code
//

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include <irmo/packet.h>

#include "connection.h"
#include "protocol.h"

// size of packet buffer (maximum packet size
// 64KiB default

#define PACKET_BUFFER_LEN 0x10000

// send a connection refused SYN-FIN packet

static void server_send_refuse(IrmoServer *server,
			       IrmoNetAddress *address,
			       char *s, ...)
{
	IrmoPacket *packet;
	char *message;
	va_list args;

	va_start(args, s);

	message = irmo_vasprintf(s, args);

	va_end(args);

	packet = irmo_packet_new();

	irmo_packet_writei16(packet, PACKET_FLAG_SYN|PACKET_FLAG_FIN);
	irmo_packet_writestring(packet, message);

	irmo_net_socket_send_packet(server->socket, address, packet);
		
	irmo_packet_free(packet);

	free(message);
}

static void server_run_syn(IrmoServer *server,
                           IrmoClient *client)
{
	IrmoPacket *sendpacket;

	if (client->state == CLIENT_DISCONNECTED) {

		// There was previously a client connected from the
		// same port. possibly the the player disconnected
		// and reconnected and the OS gave the same random
		// port. remove the old client and destroy it, so
		// that we can start a new one.

		// remove from hash tables

		irmo_hash_table_remove(client->server->clients,
                                       client->address);

		irmo_client_internal_unref(client);

	} else if (client->state == CLIENT_CONNECTING) {

                // Send a SYN/ACK reply

                sendpacket = irmo_packet_new();

                irmo_packet_writei16(sendpacket, 
                                     PACKET_FLAG_SYN|PACKET_FLAG_ACK);

                irmo_net_socket_send_packet(server->socket,
                                            client->address,
                                            sendpacket);
                        
                irmo_packet_free(sendpacket);
        }
}

static int server_check_hashes(IrmoServer *server,
                               unsigned int local_hash,
                               unsigned int server_hash)
{
	unsigned int local_hash_expected=0, server_hash_expected=0;

        // Get the expected hashes.

        if (server->client_interface != NULL) {
                local_hash_expected = irmo_interface_hash(server->client_interface);
        } else { 
                local_hash_expected = 0;
        }

        if (server->world != NULL) {
                server_hash_expected = irmo_interface_hash(server->world->iface);
        } else {
                server_hash_expected = 0;
        }

        // Check the hashes received against our local hashes.

        return local_hash == local_hash_expected
            && server_hash == server_hash_expected;
}

static void server_run_initial_syn(IrmoServer *server, 
                                   IrmoPacket *packet,
                                   IrmoNetAddress *addr)
{
	unsigned int local_hash, server_hash;
	unsigned int protocol_version;
        IrmoClient *client;

	// Read packet data
	
	if (!irmo_packet_readi16(packet, &protocol_version)) {
		return;
        }

	if (protocol_version != IRMO_PROTOCOL_VERSION) {
		server_send_refuse(server, addr,
				   "client and server side protocol versions "
				   "do not match");
		return;
	}

	if (!irmo_packet_readi32(packet, &local_hash)
	 || !irmo_packet_readi32(packet, &server_hash)) {
		// no hashes - drop

		return;
	}

        if (!server_check_hashes(server, local_hash, server_hash)) {
		server_send_refuse(server, addr,
				   "client side and server side client "
				   "interfaces do not match");
		return;
	}

	// Valid SYN!
		
        // This is the first SYN that we have received. 
	// Create a new client object.

        client = irmo_client_new(server, addr);

        // Send a response back to the new client.
 
        server_run_syn(server, client);
}

// handle syn-ack connection acknowledgements

static void server_run_synack(IrmoServer *server,
                              IrmoClient *client)
{
	if (client->state == CLIENT_CONNECTING) {
		// this is the first synack we have received
		
		client->state = CLIENT_CONNECTED;

		// create the remote world object

		if (client->server->client_interface != NULL) {
			client->world
			  = irmo_world_new(client->server->client_interface);

			// mark this as a remote world
			
			client->world->remote = 1;

			client->world->remote_client = client;
		}

		// if we are serving a world to the client,
		// send the entire current world state

		if (client->server->world != NULL) {
			irmo_client_sendq_add_state(client);
                }

		// raise callback functions for new client
		// do this after sending the state: it may create
		// new objects in the callback, in which case the
		// 'new' can be created twice

		irmo_server_raise_connect(client->server, client);
	}

	// if we are the client receiving this from the server,
	// we need to send a syn ack back so it can complete
	// its connection.

	if (server->internal_server) {
		IrmoPacket *sendpacket = irmo_packet_new();

		irmo_packet_writei16(sendpacket, 
				     PACKET_FLAG_SYN|PACKET_FLAG_ACK);

		irmo_net_socket_send_packet(server->socket,
                                            client->address,
                                            sendpacket);

		irmo_packet_free(sendpacket);
	}

	// dont do this if we're the server, or we'll get stuck in
	// an infinite loop :)
}

// run SYN FIN (disconnect)

static void server_run_synfin(IrmoServer *server,
                              IrmoPacket *packet,
                              IrmoClient *client)
{
	IrmoPacket *sendpacket;
	
	// connection refused?
	
	if (client->state == CLIENT_CONNECTING) {
		char *message;

		// read the reason message

		message = irmo_packet_readstring(packet);

		if (message != NULL) {
			irmo_connection_error(client, "connection refused (%s)", message);
		} else {
			irmo_connection_error(client, "connection refused");
                }

		client->state = CLIENT_DISCONNECTED;
	}

	// disconnect

	if (client->state == CLIENT_CONNECTED) {
		client->state = CLIENT_DISCONNECTED;
		client->connect_time = time(NULL);
		client->disconnect_wait = 1;

		irmo_client_callback_raise(&client->disconnect_callbacks,
					   client);
	}

	if (client->state == CLIENT_DISCONNECTED) {
		
		// send a syn/fin/ack to reply

		sendpacket = irmo_packet_new();
		irmo_packet_writei16(sendpacket,
				     PACKET_FLAG_SYN | PACKET_FLAG_FIN
						     | PACKET_FLAG_ACK);

		irmo_net_socket_send_packet(server->socket,
                                            client->address,
                                            sendpacket);

		irmo_packet_free(sendpacket);
	}
}

static void server_run_synfinack(IrmoClient *client)
{
	if (client->state == CLIENT_DISCONNECTING) {
		client->state = CLIENT_DISCONNECTED;
		irmo_client_callback_raise(&client->disconnect_callbacks,
					   client);
	}
}

static void server_run_packet(IrmoServer *server, 
                              IrmoPacket *packet, 
                              IrmoNetAddress *addr)
{
	unsigned int flags;
	IrmoClient *client;

	// find a client from the socket hashtable

	client = irmo_hash_table_lookup(server->clients, addr);

	// read packet header
	
	if (!irmo_packet_readi16(packet, &flags)) {
		// cant read header
		// drop packet
		
		return;
	}

	// Check for SYN
	//
	// Internal servers used as part of an IrmoConnection do not accept
	// SYN packets.

	if (!server->internal_server && flags == PACKET_FLAG_SYN) {

                // Run a different function based on whether this is the
                // initial SYN, or we already have a client.

                if (client == NULL) {
                        server_run_initial_syn(server, packet, addr);
                } else {
                        server_run_syn(server, client);
                }

		return;
	}

	if (client == NULL) {
		// no client for this yet: havent received a syn yet
		// so drop packet

		return;
	}

	// check for syn ack connection acknowledgements
	
	if (flags == (PACKET_FLAG_SYN|PACKET_FLAG_ACK)) {
		server_run_synack(server, client);
		return;
	}

	if (flags == (PACKET_FLAG_SYN|PACKET_FLAG_FIN)) {
		server_run_synfin(server, packet, client);
		return;
	}

	if (flags == (PACKET_FLAG_SYN|PACKET_FLAG_FIN|PACKET_FLAG_ACK)) {
		server_run_synfinack(client);
		return;
	}

	if (client->state != CLIENT_CONNECTED) {
		return;
        }
	
	// pass it to the protocol parsing code
	
	irmo_proto_parse_packet(packet, client, flags);
}

static void server_run_clients(IrmoServer *server)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        // Iterate over all clients

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {

                client = irmo_hash_table_iter_next(iter);

                // Run the client

                irmo_client_run(client);

                // dont remove clients which aren't disconnected

                if (client->state != CLIENT_DISCONNECTED) {
                        continue;
                }

                // if this is a client remotely disconnecting,
                // we wait a while before destroying the object
                
                if (client->disconnect_wait
                    && time(NULL) - client->connect_time < 10) {
                        continue;
                }
                
                // remove from socket list: return 1
                
                irmo_hash_table_remove(server->clients,
                                       client->address);

                irmo_client_internal_unref(client);
        }

        irmo_hash_table_iter_free(iter);
}

void irmo_server_run(IrmoServer *server)
{
        IrmoNetAddress *src_addr;
        IrmoPacket *packet;

	irmo_return_if_fail(server != NULL);

        for (;;) {
                packet = irmo_net_socket_recv_packet(server->socket,
                                                     &src_addr);

                if (packet == NULL) {
                        break;
                }

                // Successfully received a packet!  Parse the contents.

                server_run_packet(server, packet, src_addr);
                
                // Finished now; free the packet and possibly the address.
      
                irmo_packet_free(packet);
                irmo_net_address_unref(src_addr);
        }

	// run each of the clients

        server_run_clients(server);
}

