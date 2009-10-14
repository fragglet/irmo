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

// 
// UDP/IP IrmoNetModule implementation.
//

#include "arch/sysheaders.h"
#include "base/error.h"
#include "base/alloc.h"

#include "algo/algo.h"

#include <irmo/module_ip.h>

#include "socket-base.h"

//---------------------------------------------------------------------------
//
// IPv4Address class.
//
//---------------------------------------------------------------------------

typedef struct {
        IrmoNetAddress irmo_address;
        struct sockaddr_in sockaddr;
} IPv4Address;

// Addresses hashed from struct sockaddr_in to the IPv4Address.

static IrmoHashTable *address_hash_table = NULL;

// Free a IPv4Address.

static void ipv4_address_free(IrmoNetAddress *_addr)
{
        IPv4Address *addr = (IPv4Address *) _addr;

        irmo_hash_table_remove(address_hash_table, &addr->sockaddr);
        free(addr);
}

// Convert an address to a string.

static void ipv4_address_to_string(IrmoNetAddress *_addr,
                                   char *buffer, unsigned int buffer_len)
{
        IPv4Address *addr = (IPv4Address *) _addr;

        strncpy(buffer, inet_ntoa(addr->sockaddr.sin_addr), buffer_len);

        buffer[buffer_len - 1] = '\0';
}

static unsigned int ipv4_address_get_port(IrmoNetAddress *_addr)
{
        IPv4Address *addr = (IPv4Address *) _addr;

        return ntohs(addr->sockaddr.sin_port);
}

// Address class.

static IrmoNetAddressClass ipv4_address_class = {
        ipv4_address_to_string,
        ipv4_address_get_port,
        ipv4_address_free,
};

static unsigned int sockaddr_in_hash(void *data)
{
        struct sockaddr_in *addr = data;

        return addr->sin_addr.s_addr ^ addr->sin_port;
}

static int sockaddr_in_equal(void *a, void *b)
{
        struct sockaddr_in *sockaddr1 = a;
        struct sockaddr_in *sockaddr2 = b;

        return sockaddr1->sin_addr.s_addr == sockaddr2->sin_addr.s_addr
            && sockaddr1->sin_port == sockaddr2->sin_port;
}

// Look up an address in the hash table, or return a new one if
// not found.

static IPv4Address *ipv4_get_address(struct sockaddr_in *addr)
{
        IPv4Address *result;

        if (address_hash_table == NULL) {
                // First time, need to create the hash table

                address_hash_table = irmo_hash_table_new(sockaddr_in_hash,
                                                         sockaddr_in_equal);

                irmo_alloc_assert(address_hash_table != NULL);
        } else {
                // Search the hash table

                result = irmo_hash_table_lookup(address_hash_table, addr);

                if (result != NULL) {
                        return result;
                }
        }

        // Not found in the hash table, so we need to add a new one.

        result = irmo_new0(IPv4Address, 1);
        result->irmo_address.address_class = &ipv4_address_class;
        result->sockaddr.sin_family = AF_INET;
        result->sockaddr.sin_addr.s_addr = addr->sin_addr.s_addr;
        result->sockaddr.sin_port = addr->sin_port;

        irmo_alloc_assert(irmo_hash_table_insert(address_hash_table,
                                                 &result->sockaddr,
                                                 result));

        return result;
}

//---------------------------------------------------------------------------
//
// SockBaseSocket class.
//
//---------------------------------------------------------------------------

static int ipv4_send_packet(IrmoNetSocket *_sock,
                            IrmoNetAddress *_addr,
                            IrmoPacket *packet)
{
        SockBaseSocket *sock;
        IPv4Address *addr;
        ssize_t status;

        sock = (SockBaseSocket *) _sock;
        addr = (IPv4Address *) _addr;

        status = sendto(sock->sock,
                        irmo_packet_get_buffer(packet),
                        irmo_packet_get_length(packet),
                        0,
                        (struct sockaddr *) &addr->sockaddr,
                        sizeof(struct sockaddr_in));

        return status >= 0;
}

static IrmoPacket *ipv4_recv_packet(IrmoNetSocket *_sock,
                                    IrmoNetAddress **address)
{
        SockBaseSocket *sock;
        struct sockaddr_in source;
        IPv4Address *result_address;
        socklen_t source_len;
        ssize_t status;

        sock = (SockBaseSocket *) _sock;

        source_len = sizeof(source);

        status = recvfrom(sock->sock,
                          sock->recvbuf,
                          RECV_BUFFER_SIZE,
                          0,
                          (struct sockaddr *) &source,
                          &source_len);

        if (status < 0) {
                if (errno != EAGAIN) {
                        perror("ipv4_recv_packet");
                }

                // No packet received

                return NULL;
        } else {
                // Look up the address and save it

                result_address = ipv4_get_address(&source);
                *address = &result_address->irmo_address;

                // Create a packet from the received data.

                return irmo_packet_new_from(sock->recvbuf,
                                            (unsigned int) status);
        }
}

static IrmoNetSocketClass ipv4_socket_class = {
        irmo_sockbase_close,
        ipv4_send_packet,
        ipv4_recv_packet,
        irmo_sockbase_block
};

//---------------------------------------------------------------------------
//
// IPv4 net module
//
//---------------------------------------------------------------------------

static IrmoNetSocket *ipv4_open_client_sock(IrmoNetModule *module)
{
        SockBaseSocket *result;

        result = irmo_sockbase_open(&ipv4_socket_class, AF_INET);

        if (result == NULL) {
                return NULL;
        }

        return &result->irmo_socket;
}

static IrmoNetSocket *ipv4_open_server_sock(IrmoNetModule *module,
                                            unsigned int port)
{
        SockBaseSocket *result;
        struct sockaddr_in addr;
        socklen_t addr_len;
        int status;

        result = irmo_sockbase_open(&ipv4_socket_class, AF_INET);

        if (result == NULL) {
                return NULL;
        }

        // Bind to the port

        addr_len = sizeof(struct sockaddr_in);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);

        addr.sin_port = htons(((uint16_t) port));

        status = bind(result->sock, (struct sockaddr *) &addr, addr_len);

        if (status < 0) {
                irmo_error_report("ipv4_open_server_sock",
                                  "Cannot bind to port %i (%s)",
                                  port, strerror(errno));
                closesocket(result->sock);
                free(result);
                return NULL;
        }

        return &result->irmo_socket;
}

static IrmoNetAddress *ipv4_resolve_address(IrmoNetModule *module,
                                            char *address,
                                            unsigned int port)
{
	struct hostent *hp;
        struct sockaddr_in addr;
        IPv4Address *result;

	hp = gethostbyname(address);

	if (hp == NULL) {
		return NULL;
        }

        addr.sin_family = AF_INET;
        memcpy(&addr.sin_addr, hp->h_addr, sizeof(struct in_addr));
        addr.sin_port = htons((uint16_t) port);

        result = ipv4_get_address(&addr);

        return &result->irmo_address;
}

IrmoNetModule irmo_module_ipv4 = {
        ipv4_open_client_sock,
        ipv4_open_server_sock,
        ipv4_resolve_address,
};

