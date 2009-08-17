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

#include <irmo/module_ip6.h>
#include <irmo/net-module.h>
#include "net-address.h"

// Sockets API headers:

#ifdef _WIN32

#include <WinSock.h>

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#define closesocket close

#endif

#define RECV_BUFFER_SIZE 2048

//---------------------------------------------------------------------------
//
// IPv6Address class.
//
//---------------------------------------------------------------------------

typedef struct {
        IrmoNetAddress irmo_address;
        struct sockaddr_in6 sockaddr;
} IPv6Address;

// Addresses hashed from struct sockaddr_in to the IPv6Address.

static IrmoHashTable *address_hash_table = NULL;

// Free a IPv6Address.

static void ipv6_address_free(IrmoNetAddress *_addr)
{
        IPv6Address *addr = (IPv6Address *) _addr;

        irmo_hash_table_remove(address_hash_table, &addr->sockaddr);
        free(addr);
}

// Convert an address to a string.

static char *ipv6_address_to_string(IrmoNetAddress *_addr)
{
        IPv6Address *addr = (IPv6Address *) _addr;
        static char buf[INET6_ADDRSTRLEN + 10];

        buf[0] = '[';
        inet_ntop(AF_INET6, &addr->sockaddr.sin6_addr, buf + 1, sizeof(buf));
        sprintf(buf + strlen(buf), "]:%i", addr->sockaddr.sin6_port);

        return buf;
}

// Address class.

static IrmoNetAddressClass ipv6_address_class = {
        ipv6_address_to_string,
        ipv6_address_free,
};

static unsigned int sockaddr_in6_hash(void *data)
{
        struct sockaddr_in6 *addr = data;
        unsigned int i;
        unsigned int result = 0;

        // Hash the address:

        for (i=0; i<16; ++i) {
                result = (result << 5) + result + addr->sin6_addr.s6_addr[i];
        }

        // Include the port number in the result

        return result + addr->sin6_port;
}

static int sockaddr_in6_equal(void *a, void *b)
{
        struct sockaddr_in6 *sockaddr1 = a;
        struct sockaddr_in6 *sockaddr2 = b;

        return !memcmp(&sockaddr1->sin6_addr.s6_addr,
                       &sockaddr2->sin6_addr.s6_addr, 16)
            && sockaddr1->sin6_port == sockaddr2->sin6_port;
}

// Look up an address in the hash table, or return a new one if
// not found.

static IPv6Address *ipv6_get_address(struct sockaddr_in6 *addr)
{
        IPv6Address *result;

        if (address_hash_table == NULL) {
                // First time, need to create the hash table

                address_hash_table = irmo_hash_table_new(sockaddr_in6_hash,
                                                         sockaddr_in6_equal);
        } else {
                // Search the hash table

                result = irmo_hash_table_lookup(address_hash_table, addr);

                if (result != NULL) {
                        return result;
                }
        }

        // Not found in the hash table, so we need to add a new one.

        result = irmo_new0(IPv6Address, 1);
        result->irmo_address.address_class = &ipv6_address_class;

        memcpy(&result->sockaddr, addr, sizeof(struct sockaddr_in6));

        irmo_hash_table_insert(address_hash_table, &result->sockaddr,
                               result);

        return result;
}

//---------------------------------------------------------------------------
//
// IPv6Socket class.
//
//---------------------------------------------------------------------------

typedef struct {
        IrmoNetSocket irmo_socket;
        int sock;
        uint8_t *recvbuf;
} IPv6Socket;

static void ipv6_close_sock(IrmoNetSocket *_sock)
{
        IPv6Socket *sock;

        sock = (IPv6Socket *) _sock;

        closesocket(sock->sock);
        free(sock->recvbuf);
        free(sock);
}

static int ipv6_send_packet(IrmoNetSocket *_sock,
                            IrmoNetAddress *_addr,
                            IrmoPacket *packet)
{
        IPv6Socket *sock;
        IPv6Address *addr;
        int status;
       
        sock = (IPv6Socket *) _sock;
        addr = (IPv6Address *) _addr;

        status = sendto(sock->sock,
                        irmo_packet_get_buffer(packet),
                        irmo_packet_get_length(packet),
                        0,
                        (struct sockaddr *) &addr->sockaddr,
                        sizeof(struct sockaddr_in6));

        return status >= 0;
}

static IrmoPacket *ipv6_recv_packet(IrmoNetSocket *_sock,
                                    IrmoNetAddress **address)
{
        IPv6Socket *sock;
        struct sockaddr_in6 source;
        IPv6Address *result_address;
        socklen_t source_len;
        int status;

        sock = (IPv6Socket *) _sock;

        source_len = sizeof(source);

        status = recvfrom(sock->sock,
                          sock->recvbuf,
                          RECV_BUFFER_SIZE,
                          0,
                          (struct sockaddr *) &source,
                          &source_len);

        if (status < 0) {
                if (errno != EAGAIN) {
                        perror("ipv6_recv_packet");
                }

                // No packet received

                return NULL;
        } else {
                // Look up the address and save it

                result_address = ipv6_get_address(&source);
                *address = &result_address->irmo_address;

                // Create a packet from the received data.

                return irmo_packet_new_from(sock->recvbuf,
                                            (unsigned int) status);
        }
}

static int ipv6_block_set(IrmoNetSocket **handles,
                          int num_handles,
                          int timeout)
{
        fd_set socket_set;
        IPv6Socket *sock;
        struct timeval timeout_tv;
        struct timeval *timeout_param;
        int i;
        int max_sock;
        int result;

        // Build socket set and calculate the maximum socket value
 
        FD_ZERO(&socket_set);

        max_sock = 0;

        for (i=0; i<num_handles; ++i) {
                sock = (IPv6Socket *) handles[i];

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
                perror("ipv6_block_set");
                return 0;
        } else {
                return 1;
        }
}

static IrmoNetSocketClass ipv6_socket_class = {
        ipv6_close_sock,
        ipv6_send_packet,
        ipv6_recv_packet,
        ipv6_block_set,
};

//---------------------------------------------------------------------------
//
// IPv6 net module
//
//---------------------------------------------------------------------------

// Open a non-blocking UDP socket.

static IPv6Socket *ipv6_open_sock_base(void)
{
        IPv6Socket *result;
        int sock;
        int opts;

        // Open the socket

        sock = socket(AF_INET6, SOCK_DGRAM, 0);

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
                irmo_error_report("ipv6_open_sock"
                                  "cannot make socket nonblocking (%s)",
                                  strerror(errno));
                closesocket(sock);
                return NULL;
        }

        opts |= O_NONBLOCK;

        if (fcntl(sock, F_SETFL, opts) < 0) {
                irmo_error_report("ipv6_open_sock"
                                  "cannot make socket nonblocking (%s)",
                                  strerror(errno));
                closesocket(sock);
                return NULL;
        }
#endif

        // Create a socket structure

        result = irmo_new0(IPv6Socket, 1);
        result->irmo_socket.socket_class = &ipv6_socket_class;
        result->sock = sock;
        result->recvbuf = malloc(RECV_BUFFER_SIZE);

        return result;
}

static IrmoNetSocket *ipv6_open_client_sock(IrmoNetModule *module)
{
        IPv6Socket *result;

        result = ipv6_open_sock_base();

        if (result == NULL) {
                return NULL;
        }

        return &result->irmo_socket;
}

static IrmoNetSocket *ipv6_open_server_sock(IrmoNetModule *module,
                                            unsigned int port)
{
        IPv6Socket *result;
        struct sockaddr_in6 addr;
        socklen_t addr_len;
        int status;

        result = ipv6_open_sock_base();

        if (result == NULL) {
                return NULL;
        }

        // Bind to the port

        addr_len = sizeof(struct sockaddr_in6);
        addr.sin6_family = AF_INET;
        memcpy(&addr.sin6_addr, &in6addr_any, sizeof(struct in6_addr));
        addr.sin6_port = htons((uint16_t) port);

        status = bind(result->sock, (struct sockaddr *) &addr, addr_len);

        if (status < 0) {
                irmo_error_report("ipv6_open_server_sock",
                                  "Cannot bind to port %i (%s)",
                                  port, strerror(errno));
                closesocket(result->sock);
                free(result);
                return NULL;
        }

        return &result->irmo_socket;
}

static IrmoNetAddress *ipv6_resolve_address(IrmoNetModule *module,
                                            char *address,
                                            unsigned int port)
{
        IPv6Address *result;
        struct addrinfo *addresses;
        struct addrinfo *rover;

        // Get a list of addresses to which this name resolves.

        if (getaddrinfo(address, NULL, NULL, &addresses) != 0) {
                return NULL;
        }

        // Find the first IPv6 address.

        result = NULL;

        for (rover=addresses; rover != NULL; rover = rover->ai_next) {
                if (rover->ai_protocol == AF_INET6
                 && rover->ai_addrlen == sizeof(struct sockaddr_in6)) {
                        struct sockaddr_in6 sockaddr;

                        // Initialise a sockaddr structure, based on
                        // the address in the list.

                        memcpy(&sockaddr, rover->ai_addr, rover->ai_addrlen);
                        sockaddr.sin6_port = htons((uint16_t) port);

                        // Get an Irmo address structure for this sockaddr,
                        // and stop searching.

                        result = ipv6_get_address(&sockaddr);
                        break;
                }
        }

        // Free the linked list of addresses.

        freeaddrinfo(addresses);

        return &result->irmo_address;
}

IrmoNetModule irmo_module_ipv6 = {
        ipv6_open_client_sock,
        ipv6_open_server_sock,
        ipv6_resolve_address,
};

