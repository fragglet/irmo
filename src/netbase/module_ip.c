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
#include "base/error.h"
#include "base/util.h"

#include "algo/algo.h"

#include <irmo/net-module.h>

#ifndef _WIN32
#define closesocket close
#endif

#define RECV_BUFFER_SIZE 2048

// 
// UDP/IP IrmoNetModule implementation.
//

typedef struct {
        IrmoNetAddress irmo_address;
        struct sockaddr_in sockaddr;
} IPv4Address;

typedef struct {
        int sock;
        uint8_t *recvbuf;
} IPv4Socket;

// Addresses hashed from struct sockaddr_in to the IPv4Address.

static IrmoHashTable *address_hash_table = NULL;

// Free a IPv4Address.

static void ipv4_address_free(IrmoNetAddress *addr)
{
        irmo_hash_table_remove(address_hash_table, addr);
        free(addr);
}

// Address class.

static IrmoNetAddressClass ipv4_address_class = {
        ipv4_address_free,
};

static unsigned long sockaddr_in_hash(void *data)
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

        irmo_hash_table_insert(address_hash_table,
                               &result->sockaddr,
                               result);

        return result;
}

// Open a non-blocking UDP socket.

static IPv4Socket *ipv4_open_sock_base(void)
{
        IPv4Socket *result;
        int sock;
        int opts;

        // Open the socket

        sock = socket(AF_INET, SOCK_DGRAM, 0);

        if (sock < 0) {
                return NULL;
        }

        // Set non-blocking mode:

#ifdef _WIN32
        // this is how we set nonblocking under windows
        {
                int trueval=1;
                ioctlsocket(sock, FIONBIO, &trueval);
        }
#else
        // this is how we set nonblocking under unix

        opts = fcntl(sock, F_GETFL);

        if (opts < 0) {
                irmo_error_report("irmo_socket_new"
                                  "cannot make socket nonblocking (%s)",
                                  strerror(errno));
                closesocket(sock);
                return NULL;
        }

        opts |= O_NONBLOCK;

        if (fcntl(sock, F_SETFL, opts) < 0) {
                irmo_error_report("irmo_socket_new"
                                  "cannot make socket nonblocking (%s)",
                                  strerror(errno));
                closesocket(sock);
                return NULL;
        }
#endif

        // Create a socket structure

        result = irmo_new0(IPv4Socket, 1);
        result->sock = sock;
        result->recvbuf = malloc(RECV_BUFFER_SIZE);

        return result;
}

static void *ipv4_open_client_sock(IrmoNetModule *module)
{
        IPv4Socket *result;

        result = ipv4_open_sock_base();

        return result;
}

static void *ipv4_open_server_sock(IrmoNetModule *module, int port)
{
        IPv4Socket *result;
        struct sockaddr_in addr;
        socklen_t addr_len;
        int status;

        result = ipv4_open_sock_base();
        
        // Bind to the port

        addr_len = sizeof(struct sockaddr_in);
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(port);

        status = bind(result->sock, (struct sockaddr *) &addr, addr_len);

        if (status < 0) {
                irmo_error_report("ipv4_open_server_sock",
                                  "Cannot bind to port %i (%s)",
                                  port, strerror(errno));
                closesocket(result->sock);
                free(result);
                return NULL;
        }

        return result;
}

static void ipv4_close_sock(void *handle)
{
        IPv4Socket *sock;

        sock = (IPv4Socket *) handle;

        closesocket(sock->sock);
        free(sock->recvbuf);
        free(sock);
}

static IrmoNetAddress *ipv4_resolve_address(IrmoNetModule *module, char *address,
                                            int port)
{
	struct hostent *hp;
        struct sockaddr_in addr;
        IPv4Address *result;
	
	hp = gethostbyname(address);

	if (!hp) {
		return NULL;
        }

        addr.sin_family = AF_INET;
        memcpy(&addr.sin_addr.s_addr, &hp->h_addr, sizeof(struct in_addr));
        addr.sin_port = port;

        result = ipv4_get_address(&addr);

        return &result->irmo_address;
}

static int ipv4_send_packet(void *handle,
                            IrmoNetAddress *_addr,
                            IrmoPacket *packet)
{
        IPv4Socket *sock;
        IPv4Address *addr;
        int status;
       
        sock = (IPv4Socket *) handle;
        addr = (IPv4Address *) _addr;

        status = sendto(sock->sock,
                        irmo_packet_get_buffer(packet),
                        irmo_packet_get_length(packet),
                        0,
                        (struct sockaddr *) &addr->sockaddr,
                        sizeof(struct sockaddr_in));

        return status >= 0;
}

static IrmoPacket *ipv4_recv_packet(void *handle, 
                                    IrmoNetAddress **address)
{
        IPv4Socket *sock;
        struct sockaddr_in source;
        IPv4Address *result_address;
        socklen_t source_len;
        int status;

        sock = (IPv4Socket *) handle;

        source_len = sizeof(source);

        status = recvfrom(sock->sock,
                          sock->recvbuf,
                          RECV_BUFFER_SIZE,
                          0,
                          (struct sockaddr *) &source,
                          &source_len);

        if (status < 0) {
                perror("ipv4_recv_packet");
                return NULL;
        }

        result_address = ipv4_get_address(&source);

        *address = &result_address->irmo_address;

        return irmo_packet_new_from(sock->recvbuf, status);
}

static int ipv4_block_set(void **handles, int num_handles, int timeout)
{
        fd_set socket_set;
        IPv4Socket *sock;
        struct timeval timeout_tv;
        struct timeval *timeout_param;
        int i;
        int max_sock;
        int result;

        // Build socket set and calculate the maximum socket value
 
        FD_ZERO(&socket_set);

        max_sock = 0;

        for (i=0; i<num_handles; ++i) {
                sock = handles[i];

                FD_SET(sock->sock, &socket_set);

                if (sock->sock > max_sock) {
                        max_sock = sock->sock;
                }
        }

        // Calculate timeout_param

        if (timeout == 0) {
                timeout_param = NULL;
        } else {
                timeout_param = &timeout_tv;
                timeout_tv.tv_sec = timeout / 1000;
                timeout_tv.tv_usec = timeout % 1000;
        }

        result = select(max_sock + 1, &socket_set, NULL, NULL, timeout_param);

        if (result < 0) {
                perror("ipv4_block_set");
                return 0;
        } else {
                return 1;
        }
}

IrmoNetModule irmo_module_ipv4 = {
        ipv4_open_client_sock,
        ipv4_open_server_sock,
        ipv4_close_sock,
        ipv4_resolve_address,
        ipv4_send_packet,
        ipv4_recv_packet,
        ipv4_block_set,
};

