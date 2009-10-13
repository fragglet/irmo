//
// Copyright (C) 2009 Simon Howard
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

// Common code for testing a network module.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <irmo.h>
#include "netbase/net-address.h"
#include "netbase/net-socket.h"

#define TEST_PORT_NUM 9999

// Test creating and destroying sockets.

static void test_create_sockets(IrmoNetModule *module)
{
        IrmoNetSocket *server_sock;
        IrmoNetSocket *client_sock;

        server_sock = irmo_net_socket_open_bound(module, TEST_PORT_NUM);

        assert(server_sock != NULL);

        client_sock = irmo_net_socket_open_unbound(module);

        assert(client_sock != NULL);

        irmo_net_socket_close(server_sock);
        irmo_net_socket_close(client_sock);
}

static void test_resolve(IrmoNetModule *module, char *localhost_name)
{
        IrmoNetAddress *addr;
        IrmoNetAddress *addr2;

        // Check valid resolve (should always be able to resolve localhost)

        addr = irmo_net_address_resolve(module, localhost_name, TEST_PORT_NUM);

        assert(addr != NULL);

        // Invalid resolve:

        addr2 = irmo_net_address_resolve(module, "does.not.exist.example.com",
                                         TEST_PORT_NUM);

        assert(addr2 == NULL);

        // Multiple resolves give the same address pointer:

        addr2 = irmo_net_address_resolve(module, localhost_name, TEST_PORT_NUM);

        assert(addr == addr2);

        // Add some references and then remove them.

        irmo_net_address_ref(addr);
        irmo_net_address_ref(addr);
        irmo_net_address_ref(addr);
        irmo_net_address_unref(addr);
        irmo_net_address_unref(addr);
        irmo_net_address_unref(addr);

        // Finished.

        irmo_net_address_unref(addr);
        irmo_net_address_unref(addr2);
}

static void test_addr_to_string(IrmoNetModule *module,
                                char *localhost_name)
{
        IrmoNetAddress *addr;
        char buffer[64];

        addr = irmo_net_address_resolve(module, localhost_name, TEST_PORT_NUM);

        assert(addr != NULL);

        // Check the resulting string

        irmo_net_address_to_string(addr, buffer, sizeof(buffer));

        assert(!strcmp(buffer, "127.0.0.1")
            || !strcmp(buffer, "::1"));

        assert(irmo_net_address_get_port(addr) == TEST_PORT_NUM);

        irmo_net_address_unref(addr);
}

// Get the address of a client, by sending a packet to the server
// and checking the address when received.

static IrmoNetAddress *get_client_address(IrmoNetSocket *client_sock,
                                          IrmoNetAddress *server_addr,
                                          IrmoNetSocket *server_sock)
{
        IrmoPacket *packet;
        IrmoNetAddress *result = NULL;
        unsigned int i;

        // Send packet

        packet = irmo_packet_new();
        irmo_packet_writei32(packet, 9999);
        irmo_net_socket_send_packet(client_sock, server_addr, packet);
        irmo_packet_free(packet);

        // Wait for packet receive

        while (result == NULL)
        {
                assert(irmo_net_socket_block(server_sock, 1000) != 0);

                packet = irmo_net_socket_recv_packet(server_sock, &result);

                if (packet != NULL)
                {
                        assert(irmo_packet_readi32(packet, &i));
                        assert(i == 9999);
                        irmo_packet_free(packet);
                }
        }

        return result;
}

// Send packets from the source to the destination, and check that they
// arrive okay.

static void perform_transfer_test(IrmoNetSocket *src_sock,
                                  IrmoNetAddress *dest_addr,
                                  IrmoNetSocket *dest_sock,
                                  IrmoNetAddress *expected_addr)
{
        IrmoPacket *packet;
        IrmoNetAddress *addr;
        int received[100];
        unsigned int i;
        unsigned int n;

        // Send some packets to the server.

        for (i=0; i<100; ++i)
        {
                packet = irmo_packet_new();
                irmo_packet_writei32(packet, i);

                irmo_net_socket_send_packet(src_sock, dest_addr, packet);

                irmo_packet_free(packet);
        }

        // Receive packets at the destination.

        memset(received, 0, sizeof(received));
        i = 100;

        while (i > 0)
        {
                assert(irmo_net_socket_block(dest_sock, 1000) != 0);

                packet = irmo_net_socket_recv_packet(dest_sock, &addr);

                if (packet != NULL)
                {
                        assert(addr == expected_addr);

                        // Read the packet number:

                        assert(irmo_packet_readi32(packet, &n) != 0);

                        assert(received[n] == 0);
                        received[n] = 1;
                        --i;

                        irmo_packet_free(packet);
                        irmo_net_address_unref(addr);
                }
        }
}

// Create a server socket, connect to it from a client socket, send
// packets both ways and check they are received.

static void test_send_recv(IrmoNetModule *module, char *localhost_name)
{
        IrmoNetAddress *client_addr;
        IrmoNetAddress *server_addr;
        IrmoNetSocket *client_sock;
        IrmoNetSocket *server_sock;
        unsigned int i;

        // Create sockets.

        server_sock = irmo_net_socket_open_bound(module, TEST_PORT_NUM);
        client_sock = irmo_net_socket_open_unbound(module);

        assert(client_sock != NULL && server_sock != NULL);

        // Resolve the server address.

        server_addr = irmo_net_address_resolve(module, localhost_name,
                                               TEST_PORT_NUM);

        // Client address, too.

        client_addr = get_client_address(client_sock, server_addr,
                                         server_sock);

        // Perform some transfers in both directions:

        for (i=0; i<100; ++i)
        {
                perform_transfer_test(client_sock, server_addr,
                                      server_sock, client_addr);
                perform_transfer_test(server_sock, client_addr,
                                      client_sock, server_addr);
        }

        // All received ok.

        irmo_net_socket_close(client_sock);
        irmo_net_socket_close(server_sock);

        irmo_net_address_unref(client_addr);
        irmo_net_address_unref(server_addr);
}

void run_net_module_tests(IrmoNetModule *module, char *localhost_name)
{
        test_create_sockets(module);
        test_resolve(module, localhost_name);
        test_addr_to_string(module, localhost_name);
        test_send_recv(module, localhost_name);
}

