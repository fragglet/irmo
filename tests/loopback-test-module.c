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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <irmo.h>
#include <irmo/net-module.h>

#include "arch/arch-time.h"
#include "algo/queue.h"

#define NUM_LOOPBACK_PORTS 16

typedef struct _LoopbackSocket LoopbackSocket;
typedef struct _LoopbackPacketData LoopbackPacketData;

// Data associated with a packet waiting in a receive queue.

struct _LoopbackPacketData
{
        // The packet being received.

        IrmoPacket *packet;

        // Source address of the packet.

        IrmoNetAddress *source;
};

struct _LoopbackSocket
{
        // Base net socket structure.

        IrmoNetSocket irmo_socket;

        // Queue of LoopbackPacketData structures containing 
        // packets waiting to be delivered.

        IrmoQueue *recv_queue;

        // Port number where packets for this socket are delivered.

        unsigned int port_num;
};

// Loopback address structures.  There is only one "address", but
// a different structure for each port.

static IrmoNetAddress addresses[NUM_LOOPBACK_PORTS];

// Sockets bound to each port.  If NULL, no socket is bound.

static LoopbackSocket *sockets[NUM_LOOPBACK_PORTS];

//---------------------------------------------------------------------------
//
// Lookback address class.
//
//---------------------------------------------------------------------------

// Free a IPv4Address.

static void loopback_address_free(IrmoNetAddress *_addr)
{
        // no-op.
}

// Convert an address to a string.

static void loopback_address_to_string(IrmoNetAddress *_addr,
                                       char *buffer, unsigned int buffer_len)
{
        strncpy(buffer, "localhost", buffer_len);

        buffer[buffer_len - 1] = '\0';
}

static int address_to_port(IrmoNetAddress *addr, unsigned int *result)
{
        if (addr < addresses || addr > &addresses[NUM_LOOPBACK_PORTS - 1]) {
                return 0;
        }

        *result = (unsigned int) (addr - addresses);

        return 1;
}

static unsigned int loopback_address_get_port(IrmoNetAddress *addr)
{
        unsigned int port;

        if (address_to_port(addr, &port)) {
                return port;
        } else {
                return 0;
        }
}

// Address class.

static IrmoNetAddressClass loopback_address_class = {
        loopback_address_to_string,
        loopback_address_get_port,
        loopback_address_free,
};

//---------------------------------------------------------------------------
//
// LoopbackSocket class.
//
//---------------------------------------------------------------------------

static IrmoPacket *dup_packet(IrmoPacket *packet)
{
        IrmoPacket *result;
        unsigned char *buf;
        unsigned int buf_len;
        unsigned int i;

        // Get buffer.

        buf = irmo_packet_get_buffer(packet);
        buf_len = irmo_packet_get_length(packet);

        // Create a new packet and copy the contents of the buffer in.

        result = irmo_packet_new();

        for (i=0; i<buf_len; ++i) {
                irmo_packet_writei8(result, buf[i]);
        }

        return result;
}

static int loopback_send_packet(IrmoNetSocket *_sock,
                                IrmoNetAddress *addr,
                                IrmoPacket *packet)
{
        LoopbackSocket *sock = (LoopbackSocket *) _sock;
        LoopbackPacketData *packet_data;
        unsigned int port;

        if (!address_to_port(addr, &port)) {
                return 0;
        }

        // TODO: Packet loss simulation, latency, etc.

        // Duplicate the packet and insert into the receive queue.

        packet_data = malloc(sizeof(LoopbackPacketData));
        assert(packet_data != NULL);
        packet_data->packet = dup_packet(packet);
        packet_data->source = &addresses[sock->port_num];

        irmo_queue_push_tail(sock->recv_queue, packet_data);

        return 1;
}

static IrmoPacket *loopback_recv_packet(IrmoNetSocket *_sock,
                                        IrmoNetAddress **address)
{
        LoopbackSocket *sock = (LoopbackSocket *) _sock;
        LoopbackPacketData *packet_data;
        IrmoPacket *result;

        // No packets waiting?

        if (irmo_queue_is_empty(sock->recv_queue)) {
                return NULL;
        }

        // Get the first packet from the head.

        packet_data = irmo_queue_pop_head(sock->recv_queue);
        result = packet_data->packet;
        *address = packet_data->source;
        free(packet_data);

        return result;
}

int loopback_socket_block(IrmoNetSocket **handles, int num_handles, int timeout)
{
        LoopbackSocket *sock;
        unsigned int start_time;
        int i;

        // Check if packets are waiting at any of the sockets.

        for (i=0; i<num_handles; ++i) {
                sock = (LoopbackSocket *) handles[i];

                if (!irmo_queue_is_empty(sock->recv_queue)) {
                        return 1;
                }
        }

        // No? Go to sleep, then.

        start_time = irmo_get_time();

        while (irmo_get_time() < start_time + timeout) {
                // Should be a usleep() or something similar in here.
        }

        return 1;
}

static void loopback_socket_close(IrmoNetSocket *_sock)
{
        LoopbackSocket *sock = (LoopbackSocket *) _sock;
        LoopbackPacketData *packet_data;

        // Free all packets waiting for delivery.

        while (!irmo_queue_is_empty(sock->recv_queue)) {
                packet_data = irmo_queue_pop_head(sock->recv_queue);

                irmo_packet_free(packet_data->packet);
                free(packet_data);
        }

        // Unbind the socket.

        sockets[sock->port_num] = NULL;

        // Finished.

        free(sock);
}

static IrmoNetSocketClass loopback_socket_class = {
        loopback_socket_close,
        loopback_send_packet,
        loopback_recv_packet,
        loopback_socket_block
};

//---------------------------------------------------------------------------
//
// Loopback net module
//
//---------------------------------------------------------------------------

static IrmoNetSocket *loopback_open_server_sock(IrmoNetModule *module,
                                                unsigned int port)
{
        LoopbackSocket *sock;

        // Invalid port or already bound?

        if (port >= NUM_LOOPBACK_PORTS || sockets[port] != NULL) {
                return NULL;
        }

        sock = malloc(sizeof(LoopbackSocket));
        sock->irmo_socket.socket_class = &loopback_socket_class;
        sock->port_num = port;
        sock->recv_queue = irmo_queue_new();

        // Bind.

        sockets[port] = sock;

        return &sock->irmo_socket;
}

static IrmoNetSocket *loopback_open_client_sock(IrmoNetModule *module)
{
        unsigned int i;

        // Find a free socket and bind to it in the same way as
        // a server socket.

        for (i=NUM_LOOPBACK_PORTS-1; i > 0; --i) {
                if (sockets[i] == NULL) {
                        return loopback_open_server_sock(module, i);
                }
        }

        // None free?

        return NULL;
}

static IrmoNetAddress *loopback_resolve_address(IrmoNetModule *module,
                                                char *address,
                                                unsigned int port)
{
        IrmoNetAddress *result;

        if (port < NUM_LOOPBACK_PORTS) {
                result = &addresses[port];
                result->address_class = &loopback_address_class;
        } else {
                result = NULL;
        }

        return result;
}


IrmoNetModule irmo_module_loopback = {
        loopback_open_client_sock,
        loopback_open_server_sock,
        loopback_resolve_address,
};

