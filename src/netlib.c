// Network util functions

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <netdb.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <glib.h>

#include "netlib.h"

int sockaddr_len(int domain)
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

guint sockaddr_in_hash(struct sockaddr_in *addr)
{
	return addr->sin_addr.s_addr ^ addr->sin_port;
}

gint sockaddr_in_cmp(struct sockaddr_in *a,
		     struct sockaddr_in *b)
{
	return a->sin_addr.s_addr == b->sin_addr.s_addr
	    && a->sin_port == b->sin_port;
}

#ifdef USE_IPV6

guint sockaddr_in6_hash(struct sockaddr_in6 *addr)
{
	guint *a = addr->sin6_addr.in6_u.u6_addr32;
	
	return a[0] ^ a[1] ^ a[2] ^ a[3] ^ addr->sin6_port;
}

gint sockaddr_in6_cmp(struct sockaddr_in6 *a,
		      struct sockaddr_in6 *b)
{
	guint *aa = a->sin6_addr.in6_u.u6_addr32;
	guint *ba = b->sin6_addr.in6_u.u6_addr32;

	return aa[0] == ba[0] && aa[1] == ba[1]
	    && aa[2] == ba[2] && aa[3] == ba[3]
	    && a->sin6_port == b->sin6_port;
}

#endif /* #ifdef USE_IPV6 */

guint sockaddr_hash(struct sockaddr *addr)
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

gint sockaddr_cmp(struct sockaddr *a, struct sockaddr *b)
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

struct sockaddr *sockaddr_copy(struct sockaddr *addr)
{
	int len = sockaddr_len(addr->sa_family);
	struct sockaddr *cp = malloc(len);

	memcpy(cp, addr, len);

	return cp;
}

static struct sockaddr_in *sockaddr_in_for_name(gchar *name, int port)
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
	
	return addr;
}

struct sockaddr *sockaddr_for_name(int domain, gchar *name, int port)
{
	switch (domain) {
	case AF_UNSPEC:
	case AF_INET:
		return (struct sockaddr *) sockaddr_in_for_name(name, port);
		break;
#ifdef USE_IPV6
	case AF_INET6:
#endif
	}

	return NULL;
}

// $Log: not supported by cvs2svn $
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
