//
// Base socket code
//

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <glib.h>

#include "socket.h"

#define USE_IPV6

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

static int get_sockaddr_len(int domain)
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

IrmoSocket *socket_new(int domain, int port)
{
	IrmoSocket *irmosock;
	int sock;
	struct sockaddr *addr;
	int addr_len;
	
	if (domain == AF_UNSPEC)
		domain = AF_INET;

	// check for supported domains

	if (domain != AF_INET
#ifdef USE_IPV6
         || domain != AF_INET6
#endif
		) {
		fprintf(stderr,
			"socket_new: unsupported domain (%i)\n", domain);
		return NULL;
	}
	
	// try to create the socket and bind to the port

	sock = socket(domain, SOCK_DGRAM, 0);

	if (sock < 0) {
		fprintf(stderr,
			"socket_new: Can't create new datagram socket in "
			"domain %i (%s)\n",
			domain, strerror(errno));

		return NULL;
	}

	// bind

	addr_len = get_sockaddr_len(domain);
	addr = (struct sockaddr *) g_malloc0(addr_len);

	if (bind(sock, addr, addr_len) < 0) {
		fprintf(stderr,
			"socket_new: Can't bind to %i::%i (%s)\n",
			domain, port, strerror(errno));
		close(sock);
		return NULL;
	}

	// wrap it all up in an IrmoSocket object

	irmosock = g_new0(IrmoSocket, 1);
	irmosock->domain = domain;
	irmosock->sock = sock;
	irmosock->port = port;
	irmosock->servers = g_hash_table_new(g_str_hash, g_str_equal);
	irmosock->clients = g_hash_table_new((GHashFunc) sockaddr_hash,
					     (GCompareFunc) sockaddr_cmp);

	return irmosock;
}

// $Log: not supported by cvs2svn $
// Revision 1.1  2002/11/26 15:23:50  sdh300
// Initial network code
//
