//
// Base socket code
//

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <glib.h>

#include "netlib.h"
#include "packet.h"
#include "socket.h"

// size of packet buffer (maximum packet size
// 64KiB default

#define PACKET_BUFFER_LEN 0x10000

IrmoSocket *socket_new(int domain, int port)
{
	IrmoSocket *irmosock;
	int sock;
	struct sockaddr *addr;
	int addr_len;
	int opts;
	
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

	addr_len = sockaddr_len(domain);
	addr = (struct sockaddr *) g_malloc0(addr_len);

	switch (domain) {
	case AF_INET:
		((struct sockaddr_in *) addr)->sin_family = AF_INET;
		((struct sockaddr_in *) addr)->sin_addr.s_addr = INADDR_ANY;
		((struct sockaddr_in *) addr)->sin_port = htons(port);
		break;
#ifdef USE_IPV6
	case AF_INET6:
		((struct sockaddr_in6 *) addr)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *) addr)->sin6_addr = in6addr_any;
		((struct sockaddr_in6 *) addr)->sin6_port = htons(port);
		
		break;
#endif
	}
	
	if (bind(sock, addr, addr_len) < 0) {
		fprintf(stderr,
			"socket_new: Can't bind to %i::%i (%s)\n",
			domain, port, strerror(errno));
		close(sock);
		return NULL;
	}

	free(addr);

	// make socket nonblocking

	opts = fcntl(sock, F_GETFL);

	if (opts < 0) {
		fprintf(stderr,
			"socket_new: Can't fcntl(F_GETFL) (%s)\n",
			strerror(errno));
		close(sock);
		return NULL;
	}

	opts |= O_NONBLOCK;

	if (fcntl(sock, F_SETFL, opts) < 0) {
		fprintf(stderr,
			"socket_new: Can't fcntl(F_SETFL) (%s)\n",
			strerror(errno));
		close(sock);
		return NULL;
	}
	
	// wrap it all up in an IrmoSocket object

	irmosock = g_new0(IrmoSocket, 1);
	irmosock->refcount = 1;
	irmosock->domain = domain;
	irmosock->sock = sock;
	irmosock->port = port;
	irmosock->servers = g_hash_table_new(g_str_hash, g_str_equal);
	irmosock->clients = g_hash_table_new((GHashFunc) sockaddr_hash,
					     (GCompareFunc) sockaddr_cmp);

	return irmosock;
}

void socket_ref(IrmoSocket *sock)
{
	++sock->refcount;
}

void socket_unref(IrmoSocket *sock)
{
	--sock->refcount;

	if (sock->refcount <= 0) {
		
		// close socket

		close(sock->sock);
		
		// if there are no references to this socket, it follows
		// there are no servers or clients either as they
		// reference it.

		g_hash_table_destroy(sock->clients);
		g_hash_table_destroy(sock->servers);

		free(sock);
	}
}

static inline void socket_run_syn(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoServer *server;
	IrmoPacket *sendpacket;
	guint32 local_hash, server_hash;
	gchar *s;
	
	if (!packet_readi32(packet, &local_hash)
	 && !packet_readi32(packet, &server_hash)) {
		// no hashes - drop

		return;
	}

	// read server vhost name
	
	s = packet_readstring(packet);

	if (s) {
		server = g_hash_table_lookup(packet->sock->servers,
					     s);
	} else {
		// use default server

		server = packet->sock->default_server;
	}

	if (!server || local_hash != server->client_spec->hash
	    || server_hash != server->universe->spec->hash) {
		// server not found, or invalid parameters (spec hashes
		// are wrong)
		// send a refusal

		sendpacket = packet_new(2);

		packet_writei16(sendpacket, PACKET_FLAG_SYN|PACKET_FLAG_FIN);

		/* todo
		socket_sendpacket(packet->sock,
				  packet->src,
				  sendpacket);
		*/
		
		packet_free(sendpacket);
	} else {
		// valid syn
		
		// if this is the first syn we have received,
		// create a new client object
		
		if (!client) 
			client = _client_new(server, packet->src);

		// send a reply

		sendpacket = packet_new(2);

		packet_writei16(sendpacket, PACKET_FLAG_SYN|PACKET_FLAG_ACK);

		/* todo
		   socket_sendpacket(packet->sock,
		                     packet->src,
				     sendpacket);
		*/
		
		packet_free(sendpacket);
	}
}

static inline void socket_run_packet(IrmoPacket *packet)
{
	guint16 flags;
	IrmoClient *client;

	// find a client from the socket hashtable

	packet->client = client
		= g_hash_table_lookup(packet->sock->clients, packet->src);

	// read packet header
	
	if (!packet_readi16(packet, &flags)) {
		// cant read header
		// drop packet
		
		return;
	}

	packet->flags = flags;

	// check for syn

	if (flags == PACKET_FLAG_SYN) {
		socket_run_syn(packet);

		return;
	}

	if (!client) {
		// no client for this yet: havent received a syn yet
		// so drop packet

		return;
	}
	
}

void socket_run(IrmoSocket *sock)
{
	guchar buf[PACKET_BUFFER_LEN];
	struct sockaddr *addr;
	int addr_len;

	addr_len = sockaddr_len(sock->domain);
	addr = malloc(addr_len);
	
	while (1) {
		IrmoPacket packet;
		int result;
		int tmp_addr_len = addr_len;
		
		result = recvfrom(sock->sock,
				  buf,
				  PACKET_BUFFER_LEN,
				  0,
				  addr,
				  &tmp_addr_len);

		if (result < 0) {
			if (errno != EWOULDBLOCK)
				fprintf(stderr,
					"socket_run: error on receive (%s)\n",
					strerror(errno));
			
			break;
		}

		// stick it in a packet

		packet.sock = sock;
		packet.src = addr;
		packet.data = buf;
		packet.len = result;
		packet.pos = 0;

		socket_run_packet(&packet);
      	}

	free(addr);

	// send stuff too eventually
}

// $Log: not supported by cvs2svn $
// Revision 1.11  2003/02/03 20:44:20  sdh300
// move sockaddr_len into netlib
//
// Revision 1.10  2002/12/02 22:39:54  sdh300
// Fix binding to sockets (structure not initialise properly)
//
// Revision 1.9  2002/12/02 22:24:23  sdh300
// Initial socket run code/add extra data to packet objects
//
// Revision 1.8  2002/12/02 22:04:04  sdh300
// Make created sockets nonblocking
//
// Revision 1.7  2002/12/02 21:56:46  sdh300
// Fix build (compile errors)
//
// Revision 1.6  2002/12/02 21:32:51  sdh300
// reference counting for IrmoSockets
//
// Revision 1.5  2002/11/26 16:37:00  sdh300
// add missing header to remove warnings
// || test should be &&
//
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
