// Network util functions

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <glib.h>

#include "netlib.h"

guint sockaddr_in_hash(struct sockaddr_in *addr)
{
	return addr->sin_addr.s_addr;
}

gint sockaddr_in_cmp(struct sockaddr_in *a,
		     struct sockaddr_in *b)
{
	return a->sin_addr.s_addr == b->sin_addr.s_addr;
}

#ifdef USE_IPV6

guint sockaddr_in6_hash(struct sockaddr_in6 *addr)
{
	guint *a = addr->sin6_addr.in6_u.u6_addr32;
	
	return a[0] ^ a[1] ^ a[2] ^ a[3];
}

gint sockaddr_in6_cmp(struct sockaddr_in6 *a,
		      struct sockaddr_in6 *b)
{
	guint *aa = a->sin6_addr.in6_u.u6_addr32;
	guint *ba = b->sin6_addr.in6_u.u6_addr32;

	return aa[0] == ba[0] && aa[1] == ba[1]
	    && aa[2] == ba[2] && aa[3] == ba[3];
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

// $Log: not supported by cvs2svn $
