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

#include "packet.h"

#include <irmo/net-module.h>

#ifndef _WIN32
#define closesocket close
#endif

#define RECV_BUFFER_SIZE 2048

// 
// UDP/IP IrmoNetModule implementation.
//

typedef struct {
        int sock;
        uint8_t *recvbuf;
} IPv4Socket;

// All addresses map to the same struct in_addr pointer.

static IrmoHashTable *address_hash_table = NULL;

static unsigned long in_addr_hash(void *data)
{
        struct in_addr *addr = data;

        return addr->s_addr;
}

static int in_addr_equal(void *a, void *b)
{
        struct in_addr *addr1 = a;
        struct in_addr *addr2 = b;

        return addr1->s_addr == addr2->s_addr;
}

// Look up an address in the hash table, or return a new one if
// not found.

static struct in_addr *ipv4_get_address(struct in_addr *addr)
{
        struct in_addr *result;

        if (address_hash_table == NULL) {
                // First time, need to create the hash table

                address_hash_table = irmo_hash_table_new(in_addr_hash,
                                                         in_addr_equal);
        } else {
                // Search the hash table

                result = irmo_hash_table_lookup(address_hash_table, addr);

                if (result != NULL) {
                        return result;
                }
        }

        // Not found in the hash table, so we need to add a new one.

        result = irmo_new0(struct in_addr, 1);
        result->s_addr = addr->s_addr;

        irmo_hash_table_insert(address_hash_table, result, result);

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

static void *ipv4_resolve_address(IrmoNetModule *module, char *address)
{
	struct hostent *hp;
	struct in_addr addr;
	
	hp = gethostbyname(address);

	if (!hp) {
		return NULL;
        }

        memcpy(&addr, &hp->h_addr, sizeof(struct in_addr));

        return ipv4_get_address(&addr);
}

static void ipv4_free_address(void *address)
{
        irmo_hash_table_remove(address_hash_table, address);
        free(address);
}

static int ipv4_send_packet(void *handle, void *_address,
                            int port, IrmoPacket *packet)
{
        IPv4Socket *sock;
        struct in_addr *address;
        struct sockaddr_in dest;
        int status;
       
        sock = (IPv4Socket *) handle;
        address = (struct in_addr *) _address;

        dest.sin_family = AF_INET;
        dest.sin_addr.s_addr = address->s_addr;
        dest.sin_port = htons(port);

        status = sendto(sock->sock,
                        packet->data,
                        packet->len,
                        0,
                        (struct sockaddr *) &dest,
                        sizeof(struct sockaddr_in));

        return status >= 0;
}

static IrmoPacket *ipv4_recv_packet(void *handle, void **address, int *port)
{
        IPv4Socket *sock;
        struct sockaddr_in source;
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

        *address = ipv4_get_address(&source.sin_addr);
        *port = ntohs(source.sin_port);

        return irmo_packet_new_from(sock->recvbuf, status);
}

IrmoNetModule irmo_module_ipv4 = {
        ipv4_open_client_sock,
        ipv4_open_server_sock,
        ipv4_close_sock,
        ipv4_resolve_address,
        ipv4_free_address,
        ipv4_send_packet,
        ipv4_recv_packet,
};

