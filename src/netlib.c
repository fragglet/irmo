// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

// Network util functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

static guint sockaddr_in_hash(struct sockaddr_in *addr)
{
	return addr->sin_addr.s_addr ^ addr->sin_port;
}

static gint sockaddr_in_cmp(struct sockaddr_in *a,
			  struct sockaddr_in *b)
{
	return a->sin_addr.s_addr == b->sin_addr.s_addr
	    && a->sin_port == b->sin_port;
}

#ifdef USE_IPV6

static guint sockaddr_in6_hash(struct sockaddr_in6 *addr)
{
	guint32 *a = (guint32 *) addr->sin6_addr.s6_addr;
	
	return a[0] ^ a[1] ^ a[2] ^ a[3] ^ addr->sin6_port;
}

static gint sockaddr_in6_cmp(struct sockaddr_in6 *a,
			   struct sockaddr_in6 *b)
{
	return !memcmp(&a->sin6_addr, &b->sin6_addr, sizeof(struct in6_addr))
	    && a->sin6_port == b->sin6_port;
}

#endif /* #ifdef USE_IPV6 */

guint irmo_sockaddr_hash(struct sockaddr *addr)
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

gint irmo_sockaddr_cmp(struct sockaddr *a, struct sockaddr *b)
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

static struct sockaddr *sockaddr_in_for_name(gchar *name, int port)
{
	struct hostent *hp;
	struct sockaddr_in *addr;
	
	hp = gethostbyname(name);

	if (!hp)
		return NULL;

	addr = g_new0(struct sockaddr_in, 1);
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	memcpy(&addr->sin_addr, hp->h_addr, hp->h_length);
	
	return (struct sockaddr *) addr;
}

#ifdef USE_IPV6

static struct sockaddr *sockaddr_in6_for_name(gchar *name, int port)
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
		addr = g_new0(struct sockaddr_in6, 1);

		memcpy(addr, info->ai_addr, sizeof(struct sockaddr_in6));

		freeaddrinfo(info);

		addr->sin6_port = htons(port);

		return (struct sockaddr *) addr;
	}

	return NULL;
}

#endif

struct sockaddr *irmo_sockaddr_for_name(IrmoSocketDomain domain, 
					gchar *name, int port)
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

#define USEC_MAX 1000000

void irmo_timeval_add(GTimeVal *a, GTimeVal *b, GTimeVal *result)
{
	result->tv_sec = a->tv_sec + b->tv_sec;
	result->tv_usec = a->tv_usec + b->tv_usec;
	
	if (result->tv_usec >= USEC_MAX) {
		++result->tv_sec;
		result->tv_usec -= USEC_MAX;
	}
}

void irmo_timeval_sub(GTimeVal *a, GTimeVal *b, GTimeVal *result)
{
	result->tv_sec = a->tv_sec - b->tv_sec;

	if (a->tv_usec >= b->tv_usec) {
		result->tv_usec = a->tv_usec - b->tv_usec;
	} else {
		--result->tv_sec;
		result->tv_usec = USEC_MAX - (b->tv_usec - a->tv_usec);
	}
}

int irmo_timeval_cmp(GTimeVal *a, GTimeVal *b)
{
	if (a->tv_sec == b->tv_sec) {
		return a->tv_usec < b->tv_usec ? -1 :
			a->tv_usec > b->tv_usec ? 1 : 0;
	}

	return a->tv_sec < b->tv_sec ? -1 :
		a->tv_sec > b->tv_sec ? 1 : 0;
}

int irmo_timeval_to_ms(GTimeVal *a)
{
	return a->tv_sec + (a->tv_usec / 1000);
}

void irmo_timeval_from_ms(int ms, GTimeVal *time)
{
	time->tv_sec = ms / 1000;
	time->tv_usec = (ms % 1000) * 1000;
}

// $Log$
// Revision 1.7  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.6  2003/11/18 19:32:19  fraggle
// Use GTimeVal instead of struct timeval
//
// Revision 1.5  2003/11/18 18:14:46  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.4  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.3  2003/08/26 14:57:31  fraggle
// Remove AF_* BSD sockets dependency from Irmo API
//
// Revision 1.2  2003/08/06 16:15:18  fraggle
// IPv6 support
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.7  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.6  2003/03/18 20:55:46  sdh300
// Initial round trip time measurement
//
// Revision 1.5  2003/02/06 01:58:16  sdh300
// Abstracted address resolve function
//
// Revision 1.4  2003/02/03 20:54:01  sdh300
// sockaddr copy function
//
// Revision 1.3  2003/02/03 20:44:20  sdh300
// move sockaddr_len into netlib
//
// Revision 1.2  2002/11/26 16:31:27  sdh300
// oops! need to hash by port number as well as address
//
// Revision 1.1  2002/11/26 16:23:27  sdh300
// Split off sockaddr hash functions to a seperate netlib module
//
