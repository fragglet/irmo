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

// Network util functions

#include "sysheaders.h"

#include "netlib.h"

int irmo_sockaddr_len(int domain)
{
        switch (domain) {
        case AF_INET:
                return sizeof(struct sockaddr_in);
#ifdef USE_IPV6
        case AF_INET6:
                return sizeof(struct sockaddr_in6);
#endif
        }

        return 0;
}

static uint32_t sockaddr_in_hash(struct sockaddr_in *addr)
{
	return addr->sin_addr.s_addr ^ addr->sin_port;
}

static int sockaddr_in_cmp(struct sockaddr_in *a,
			  struct sockaddr_in *b)
{
	return a->sin_addr.s_addr == b->sin_addr.s_addr
	    && a->sin_port == b->sin_port;
}

#ifdef USE_IPV6

static uint32_t sockaddr_in6_hash(struct sockaddr_in6 *addr)
{
	uint32_t *a = (uint32_t *) addr->sin6_addr.s6_addr;
	
	return a[0] ^ a[1] ^ a[2] ^ a[3] ^ addr->sin6_port;
}

static int sockaddr_in6_cmp(struct sockaddr_in6 *a,
			   struct sockaddr_in6 *b)
{
	return !memcmp(&a->sin6_addr, &b->sin6_addr, sizeof(struct in6_addr))
	    && a->sin6_port == b->sin6_port;
}

#endif /* #ifdef USE_IPV6 */

unsigned int irmo_sockaddr_hash(struct sockaddr *addr)
{
	switch (addr->sa_family) {
	case AF_INET:
		return sockaddr_in_hash((struct sockaddr_in *) addr);
#ifdef USE_IPV6
	case AF_INET6:
		return sockaddr_in6_hash((struct sockaddr_in6 *) addr);
#endif
	}

	return 0;
}

int irmo_sockaddr_cmp(struct sockaddr *a, struct sockaddr *b)
{
	switch (a->sa_family) {
	case AF_INET:
		return sockaddr_in_cmp((struct sockaddr_in *) a,
				       (struct sockaddr_in *) b);
#ifdef USE_IPV6
	case AF_INET6:
		return sockaddr_in6_cmp((struct sockaddr_in6 *) a,
				        (struct sockaddr_in6 *) b);
#endif
	}

	return 0;
}

struct sockaddr *irmo_sockaddr_copy(struct sockaddr *addr)
{
	int len = irmo_sockaddr_len(addr->sa_family);
	struct sockaddr *cp = malloc(len);

	memcpy(cp, addr, len);

	return cp;
}

static struct sockaddr *sockaddr_in_for_name(char *name, int port)
{
	struct hostent *hp;
	struct sockaddr_in *addr;
	
	hp = gethostbyname(name);

	if (!hp)
		return NULL;

	addr = irmo_new0(struct sockaddr_in, 1);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);
	
	return (struct sockaddr *) addr;
}

#ifdef USE_IPV6

static struct sockaddr *sockaddr_in6_for_name(char *name, int port)
{
	struct hostent *hp;
	struct addrinfo *info;
	struct addrinfo hints;
	struct sockaddr_in6 *addr;

	hints.ai_family = PF_INET6;
	hints.ai_flags = 0;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = 0;
        
	if (getaddrinfo(name, "0", &hints, &info) == 0 && info) {
		addr = irmo_new0(struct sockaddr_in6, 1);

		memcpy(addr, info->ai_addr, sizeof(struct sockaddr_in6));

		freeaddrinfo(info);

		addr->sin6_port = htons(port);

		return (struct sockaddr *) addr;
	}

	return NULL;
}

#endif

struct sockaddr *irmo_sockaddr_for_name(IrmoSocketDomain domain, 
					char *name, int port)
{
	switch (domain) {
	case IRMO_SOCKET_AUTO:
	case IRMO_SOCKET_IPV4:
		return sockaddr_in_for_name(name, port);
#ifdef USE_IPV6
	case IRMO_SOCKET_IPV6:
		return sockaddr_in6_for_name(name, port);
#endif
	}

	return NULL;
}

