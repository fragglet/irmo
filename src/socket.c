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
#include "protocol.h"
#include "socket.h"

// size of packet buffer (maximum packet size
// 64KiB default

#define PACKET_BUFFER_LEN 0x10000

// socket send function

void irmo_socket_sendpacket(IrmoSocket *sock, struct sockaddr *dest,
			    IrmoPacket *packet)
{
	int result;

	result = sendto(sock->sock,
			packet->data,
			packet->len,
			0,
			dest,
			sockaddr_len(dest->sa_family));

	if (result < 0) {
		irmo_error_report("irmo_socket_sendpacket",
				  "error sending packet (%s)",
				  strerror(errno));
	}
}

// shared constructor
// this does not bind to a port

static IrmoSocket *_socket_new(int domain)
{
	IrmoSocket *irmosock;
	int sock;
	int opts;
	
	if (domain == AF_UNSPEC)
		domain = AF_INET;

	// check for supported domains

	if (domain != AF_INET
#ifdef USE_IPV6
         && domain != AF_INET6
#endif
		) {
		irmo_error_report("irmo_socket_new",
				  "unsupported address domain (%i)", domain);
		return NULL;
	}
	
	// try to create the socket and bind to the port

	sock = socket(domain, SOCK_DGRAM, 0);

	if (sock < 0) {
		irmo_error_report("irmo_socket_new",
				  "cannot create new datagram socket (%s)",
				  strerror(errno));

		return NULL;
	}

	// make socket nonblocking

	opts = fcntl(sock, F_GETFL);

	if (opts < 0) {
		irmo_error_report("irmo_socket_new"
				  "cannot make socket nonblocking (%s)",
				  strerror(errno));
		close(sock);
		return NULL;
	}

	opts |= O_NONBLOCK;

	if (fcntl(sock, F_SETFL, opts) < 0) {
		irmo_error_report("irmo_socket_new"
				  "cannot make socket nonblocking (%s)",
				  strerror(errno));
		close(sock);
		return NULL;
	}
	
	// wrap it all up in an IrmoSocket object

	irmosock = g_new0(IrmoSocket, 1);
	irmosock->refcount = 1;
	irmosock->domain = domain;
	irmosock->sock = sock;
	irmosock->servers = g_hash_table_new(g_str_hash, g_str_equal);
	irmosock->clients = g_hash_table_new((GHashFunc) sockaddr_hash,
					     (GCompareFunc) sockaddr_cmp);

	return irmosock;
}

void irmo_socket_ref(IrmoSocket *sock)
{
	g_return_if_fail(sock != NULL);
	
	++sock->refcount;
}

void irmo_socket_unref(IrmoSocket *sock)
{
	g_return_if_fail(sock != NULL);
	
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

// create a socket for clients, unbound

IrmoSocket *irmo_socket_new_unbound(int domain)
{
	IrmoSocket *sock = _socket_new(domain);

	if (!sock)
		return NULL;

	sock->type = SOCKET_CLIENT;
	sock->port = -1;
	
	return sock;
}

// create a socket for servers, bound to a port

IrmoSocket *irmo_socket_new(int domain, int port)
{
	IrmoSocket *sock;
	struct sockaddr *addr;
	int addr_len;

	sock = _socket_new(domain);
	
	if (!sock)
		return NULL;
	
	// try to bind to the port

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
	
	if (bind(sock->sock, addr, addr_len) < 0) {
		irmo_error_report("irmo_socket_new",
				  "cannot bind socket to port %i (%s)",
				  port, strerror(errno));
		irmo_socket_unref(sock);
		return NULL;
	}

	free(addr);

	// bound successfully

	sock->type = SOCKET_SERVER;
	sock->port = port;
	
	return sock;
}

static inline void socket_run_syn(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoServer *server;
	IrmoPacket *sendpacket;
	guint32 local_hash, server_hash;
	guint32 local_hash_expected=0, server_hash_expected=0;
	gchar *s;

	// if this is a client socket, dont let people connect
	// to us!

	if (packet->sock->type == SOCKET_CLIENT)
		return;

	// once client is connected, do not allow more SYNs

	if (client && client->state != CLIENT_CONNECTING)
		return;
	
	// read packet data
	
	if (!packet_readi32(packet, &local_hash)
	 || !packet_readi32(packet, &server_hash)) {
		// no hashes - drop

		return;
	}

	// read server vhost name
	
	s = packet_readstring(packet);

	if (s) {
		server = g_hash_table_lookup(packet->sock->servers,
					     s);
	} else {
		server = NULL;
	}

	if (!server) { 
		// try default server if vhost not found or none 
		// specified

		server = packet->sock->default_server;
	}

	if (server) {
		// if there is a server, get the hashes for the interface
		// specs

		local_hash_expected 
			= server->client_spec ? server->client_spec->hash : 0;
		server_hash_expected 
			= server->universe ? server->universe->spec->hash : 0;
	}

	if (!server || local_hash != local_hash_expected
	    || server_hash != server_hash_expected) {
		// server not found, or invalid parameters (spec hashes
		// are wrong)
		// send a refusal

		sendpacket = packet_new(2);

		packet_writei16(sendpacket, PACKET_FLAG_SYN|PACKET_FLAG_FIN);

		irmo_socket_sendpacket(packet->sock,
				       packet->src,
				       sendpacket);
		
		packet_free(sendpacket);
	} else {
		// valid syn
		
		// if this is the first syn we have received,
		// create a new client object

		if (!client) {
			client = irmo_client_new(server, packet->src);
		}
		
		// send a reply

		sendpacket = packet_new(2);

		packet_writei16(sendpacket, PACKET_FLAG_SYN|PACKET_FLAG_ACK);

		irmo_socket_sendpacket(packet->sock,
				       packet->src,
				       sendpacket);
		
		packet_free(sendpacket);
	}
}

// handle syn-ack connection acknowledgements

static inline void socket_run_synack(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	
	if (client->state == CLIENT_CONNECTING) {
		// this is the first synack we have received
		
		client->state = CLIENT_CONNECTED;

		// create the remote universe object

		if (client->server->client_spec) {
			client->universe
			  = irmo_universe_new(client->server->client_spec);

			// mark this as a remote universe
			
			client->universe->remote = TRUE;

			client->universe->remote_client = client;
		}

		// if we are serving a universe to the client,
		// send the entire current universe state

		if (client->server->universe)
			irmo_client_sendq_add_state(client);

		// raise callback functions for new client
		// do this after sending the state: it may create
		// new objects in the callback, in which case the
		// 'new' can be created twice

		irmo_server_raise_connect(client->server, client);
	}

	// if we are the client receiving this from the server,
	// we need to send a syn ack back so it can complete
	// its connection.

	if (packet->sock->type == SOCKET_CLIENT) {
		IrmoPacket *sendpacket = packet_new(2);

		packet_writei16(sendpacket, PACKET_FLAG_SYN|PACKET_FLAG_ACK);

		irmo_socket_sendpacket(packet->sock,
				       packet->src,
				       sendpacket);

		packet_free(sendpacket);
	}

	// dont do this if we're the server, or we'll get stuck in
	// an infinite loop :)
}

// run SYN FIN (disconnect)

static inline void socket_run_synfin(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoPacket *sendpacket;
	
	// connection refused?
	
	if (client->state == CLIENT_CONNECTING) {
		client->state = CLIENT_DISCONNECTED;
	}

	// disconnect

	if (client->state == CLIENT_CONNECTED) {
		client->state = CLIENT_DISCONNECTED;
		client->connect_time = time(NULL);
		client->disconnect_wait = TRUE;

		irmo_client_callback_raise(client->disconnect_callbacks,
					   client);
	}

	if (client->state == CLIENT_DISCONNECTED) {
		
		// send a syn/fin/ack to reply

		sendpacket = packet_new(2);
		packet_writei16(sendpacket,
				PACKET_FLAG_SYN | PACKET_FLAG_FIN
				| PACKET_FLAG_ACK);

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       sendpacket);

		packet_free(sendpacket);
	}
}

static inline void socket_run_synfinack(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	
	if (client->state == CLIENT_DISCONNECTING) {
		client->state = CLIENT_DISCONNECTED;
		irmo_client_callback_raise(client->disconnect_callbacks,
					   client);
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

	// check for syn ack connection acknowledgements
	
	if (flags == (PACKET_FLAG_SYN|PACKET_FLAG_ACK)) {
		socket_run_synack(packet);
		return;
	}

	if (flags == (PACKET_FLAG_SYN|PACKET_FLAG_FIN)) {
		socket_run_synfin(packet);
		return;
	}

	if (flags == (PACKET_FLAG_SYN|PACKET_FLAG_FIN|PACKET_FLAG_ACK)) {
		socket_run_synfinack(packet);
		return;
	}
	if (client->state != CLIENT_CONNECTED)
		return;
	
	// pass it to the protocol parsing code
	
	proto_parse_packet(packet);
}

static gboolean socket_run_client(gpointer key, IrmoClient *client,
				  gpointer user_data)
{
	irmo_client_run(client);

	// if this client is dead and nothing is watching it,
	// garbage collect

	if (client->state != CLIENT_DISCONNECTED)
		return FALSE;

	if (client->refcount > 0)
		return FALSE;

	// if this is a client remotely disconnecting,
	// we wait a while before destroying the object
	
	if (client->disconnect_wait
	    && time(NULL) - client->connect_time < 10)
		return FALSE;
	
	// remove from server list
	
	g_hash_table_remove(client->server->clients,
			    key);
	
	// destroy client object
	
	irmo_client_destroy(client);
	
	// remove from socket list: return TRUE
	
	return TRUE;
}

void irmo_socket_run(IrmoSocket *sock)
{
	guchar buf[PACKET_BUFFER_LEN];
	struct sockaddr *addr;
	int addr_len;

	g_return_if_fail(sock != NULL);
	
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
				irmo_error_report("irmo_socket_run",
						  "error on receive (%s)",
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

	// run each of the clients
	
	g_hash_table_foreach_remove(sock->clients,
				    (GHRFunc) socket_run_client, NULL);
}

// $Log$
// Revision 1.2  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.41  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.40  2003/04/25 01:26:19  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.39  2003/04/25 00:17:28  sdh300
// Remove unneccesary leading underscores from variables in IrmoClient
//
// Revision 1.38  2003/03/17 18:08:40  sdh300
// Remove debug messages
//
// Revision 1.37  2003/03/17 17:34:27  sdh300
// Add disconnect callbacks for clients
//
// Revision 1.36  2003/03/17 16:48:23  sdh300
// Add ability to disconnect from servers and to disconnect clients
//
// Revision 1.35  2003/03/17 15:45:01  sdh300
// Some extra sanity checks before parsing packets
//
// Revision 1.34  2003/03/14 17:33:25  sdh300
// Fix bug caused by changes in previous commit
//
// Revision 1.33  2003/03/14 16:53:45  sdh300
// Add structure member for source client for remote universes
//
// Revision 1.32  2003/03/12 18:56:25  sdh300
// Only call callback functions when existing objects have already been
// added to sendq. This is to stop multiple news on new objects created
// in connect callback functions
//
// Revision 1.31  2003/03/08 20:02:19  sdh300
// Fix bug in compile
//
// Revision 1.30  2003/03/07 14:31:19  sdh300
// Callback functions for watching new client connects
//
// Revision 1.29  2003/03/07 12:31:50  sdh300
// Add protocol.h
//
// Revision 1.28  2003/03/07 12:17:18  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.27  2003/03/06 21:29:05  sdh300
// On connect, send the entire universe state to the client
//
// Revision 1.26  2003/03/06 20:53:16  sdh300
// Checking of remote flag for universe objects
//
// Revision 1.25  2003/03/05 17:37:52  sdh300
// Add initial packet parsing code
//
// Revision 1.24  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.23  2003/02/18 20:04:40  sdh300
// Automatically increase size of packets when writing
//
// Revision 1.22  2003/02/18 17:49:17  sdh300
// understand SYN FIN responses (connection refused on connect)
//
// Revision 1.21  2003/02/16 23:41:27  sdh300
// Reference counting for client and server objects
//
// Revision 1.20  2003/02/16 23:37:28  sdh300
// Fix bug lookup up hashes when server not found
//
// Revision 1.19  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.18  2003/02/06 16:30:25  sdh300
// Create universe object when connection is established
//
// Revision 1.17  2003/02/06 02:42:38  sdh300
// Before checking hashes on connect, check for no universe served
//
// If the host we are connecting to is not found in the server hashtable,
// try the default server if there is one
//
// Revision 1.16  2003/02/06 02:09:17  sdh300
// Initial connection code
//
// Revision 1.15  2003/02/06 01:58:39  sdh300
// Security for client sockets (dont allow connections to client sockets)
//
// Revision 1.14  2003/02/06 01:01:59  sdh300
// remove test code accidentally left in
//
// Revision 1.13  2003/02/06 00:59:25  sdh300
// Split up socket constructor function, so that unbound sockets can
// be created for clients
//
// Revision 1.12  2003/02/03 21:13:55  sdh300
// establish connections from connecting clients
//
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
