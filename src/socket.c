//
// Base socket code
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <glib.h>

#include "netlib.h"
#include "socket.h"

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
         && domain != AF_INET6
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

	free(addr);
	
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
// Revision 1.4  2002/11/26 16:32:45  sdh300
// forget to free address after use
//
// Revision 1.3  2002/11/26 16:23:28  sdh300
// Split off sockaddr hash functions to a seperate netlib module
//
// Revision 1.2  2002/11/26 15:32:11  sdh300
// store port number in IrmoSocket objects as well
//
// Revision 1.1  2002/11/26 15:23:50  sdh300
// Initial network code
//
