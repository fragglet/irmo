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

#include "sysheaders.h"

#include "connection.h"
#include "error.h"
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
			irmo_sockaddr_len(dest->sa_family));

	if (result < 0) {
		irmo_error_report("irmo_socket_sendpacket",
				  "error sending packet (%s)",
				  strerror(errno));
	}
}

static int socket_type_to_domain(IrmoSocketDomain type)
{
	switch (type) {
	case IRMO_SOCKET_AUTO:
		return AF_UNSPEC;
	case IRMO_SOCKET_IPV4:
		return AF_INET;
#ifdef USE_IPV6
	case IRMO_SOCKET_IPV6:
		return AF_INET6;
#endif
	default:
		return -1;
	}
}

// shared constructor
// this does not bind to a port

static IrmoSocket *_socket_new(IrmoSocketDomain type)
{
	IrmoSocket *irmosock;
	int sock;
	int domain;
	int opts;

	if (type == IRMO_SOCKET_AUTO)
		type = IRMO_SOCKET_IPV4;

	// check for supported domains

	if (type != IRMO_SOCKET_IPV4
#ifdef USE_IPV6
         && type != IRMO_SOCKET_IPV6
#endif
		) {
		irmo_error_report("irmo_socket_new",
				  "unsupported socket type(%i)", type);
		return NULL;
	}
	
	// try to create the socket and bind to the port

	domain = socket_type_to_domain(type);
	
	sock = socket(domain, SOCK_DGRAM, 0);

	if (sock < 0) {
		irmo_error_report("irmo_socket_new",
				  "cannot create new datagram socket (%s)",
				  strerror(errno));

		return NULL;
	}

	// make socket nonblocking

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
	
	// wrap it all up in an IrmoSocket object

	irmosock = irmo_new0(IrmoSocket, 1);
	irmosock->refcount = 1;
	irmosock->domain = type;
	irmosock->sock = sock;

	return irmosock;
}

void irmo_socket_ref(IrmoSocket *sock)
{
	irmo_return_if_fail(sock != NULL);
	
	++sock->refcount;
}

void irmo_socket_shutdown(IrmoSocket *sock)
{
	irmo_return_if_fail(sock->shutdown != 1);
	
	// close socket

	closesocket(sock->sock);
		
	sock->shutdown = 1;
}

void irmo_socket_unref(IrmoSocket *sock)
{
	irmo_return_if_fail(sock != NULL);
	
	--sock->refcount;

	if (sock->refcount <= 0) {

		if (!sock->shutdown)
			irmo_socket_shutdown(sock);

		free(sock);
	}
}

// create a socket for clients, unbound

IrmoSocket *irmo_socket_new_unbound(IrmoSocketDomain type)
{
	IrmoSocket *sock = _socket_new(type);

	if (!sock)
		return NULL;

	sock->type = SOCKET_CLIENT;
	sock->port = -1;
	
	return sock;
}

// create a socket for servers, bound to a port

IrmoSocket *irmo_socket_new_bound(IrmoSocketDomain domain, int port)
{
	IrmoSocket *sock;
	struct sockaddr *addr;
	int addr_len;
	int result;

	sock = _socket_new(domain);
	
	if (!sock)
		return NULL;
	
	// try to bind to the port

	addr_len = irmo_sockaddr_len(socket_type_to_domain(sock->domain));
	addr = (struct sockaddr *) irmo_malloc0(addr_len);

	switch (sock->domain) {
	case IRMO_SOCKET_IPV4:
		((struct sockaddr_in *) addr)->sin_family = AF_INET;
		((struct sockaddr_in *) addr)->sin_addr.s_addr = INADDR_ANY;
		((struct sockaddr_in *) addr)->sin_port = htons(port);
		break;
#ifdef USE_IPV6
	case IRMO_SOCKET_IPV6:
		((struct sockaddr_in6 *) addr)->sin6_family = AF_INET6;
		((struct sockaddr_in6 *) addr)->sin6_addr = in6addr_any;
		((struct sockaddr_in6 *) addr)->sin6_port = htons(port);
		
		break;
#endif
	}
	
	result = bind(sock->sock, addr, addr_len);

	free(addr);

	if (result < 0) {
		irmo_error_report("irmo_socket_new",
				  "cannot bind socket to port %i (%s)",
				  port, strerror(errno));
		irmo_socket_unref(sock);
		return NULL;
	}

	// bound successfully

	sock->type = SOCKET_SERVER;
	sock->port = port;
	
	return sock;
}

// send a connection refused SYN-FIN packet

static void socket_send_refuse(IrmoSocket *sock,
			       struct sockaddr *addr,
			       char *s, ...)
{
	IrmoPacket *packet;
	char *message;
	va_list args;

	va_start(args, s);

	message = irmo_vasprintf(s, args);

	va_end(args);

	packet = irmo_packet_new();

	irmo_packet_writei16(packet, PACKET_FLAG_SYN|PACKET_FLAG_FIN);
	irmo_packet_writestring(packet, message);

	irmo_socket_sendpacket(sock, addr, packet);
		
	irmo_packet_free(packet);

	free(message);
}

static void socket_run_syn(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoServer *server;
	IrmoPacket *sendpacket;
	unsigned int local_hash, server_hash;
	unsigned int protocol_version;
	unsigned int local_hash_expected=0, server_hash_expected=0;
	char *s;

	// if this is a client socket, dont let people connect
	// to us!

	if (packet->sock->type == SOCKET_CLIENT)
		return;

	if (client && client->state == CLIENT_DISCONNECTED) {

		// there was previously a client connected from the
		// same port. possibly the the player disconnected
		// and reconnected and the OS gave the same random
		// port. remove the old client and destroy it, so
		// that we can start a new one.

		// remove from hash tables

		irmo_hash_table_remove(client->server->clients,
				    client->addr);

		irmo_client_internal_unref(client);

		client = NULL;
	}

	// once client is connected, do not allow more SYNs

	if (client && client->state != CLIENT_CONNECTING)
		return;
	
	// read packet data
	
	if (!irmo_packet_readi16(packet, &protocol_version))
		return;

	if (protocol_version != IRMO_PROTOCOL_VERSION) {
		socket_send_refuse(packet->sock, packet->src,
				   "client and server side protocol versions "
				   "do not match");
		return;
	}

	if (!irmo_packet_readi32(packet, &local_hash)
	 || !irmo_packet_readi32(packet, &server_hash)) {
		// no hashes - drop

		return;
	}

	server = packet->sock->server;

	// if there is a server, get the hashes for the interface
	// specs

	local_hash_expected 
		= server->client_spec ? server->client_spec->hash : 0;
	server_hash_expected 
		= server->world ? server->world->spec->hash : 0;

	if (local_hash != local_hash_expected) {
		socket_send_refuse(packet->sock, packet->src,
				   "client side and server side client "
				   "interface specifications do not match");
		return;
	}

	if (server_hash != server_hash_expected) {
		socket_send_refuse(packet->sock, packet->src,
				   "client side and server side server "
				   "interface specifications do not match");
		return;
	}

	// valid syn
		
	// if this is the first syn we have received,
	// create a new client object

	if (!client) {
		client = irmo_client_new(server, packet->src);
	}
		
	// send a reply

	sendpacket = irmo_packet_new();

	irmo_packet_writei16(sendpacket, 
			     PACKET_FLAG_SYN|PACKET_FLAG_ACK);

	irmo_socket_sendpacket(packet->sock,
			       packet->src,
			       sendpacket);
		
	irmo_packet_free(sendpacket);
}

// handle syn-ack connection acknowledgements

static void socket_run_synack(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	
	if (client->state == CLIENT_CONNECTING) {
		// this is the first synack we have received
		
		client->state = CLIENT_CONNECTED;

		// create the remote world object

		if (client->server->client_spec) {
			client->world
			  = irmo_world_new(client->server->client_spec);

			// mark this as a remote world
			
			client->world->remote = 1;

			client->world->remote_client = client;
		}

		// if we are serving a world to the client,
		// send the entire current world state

		if (client->server->world)
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
		IrmoPacket *sendpacket = irmo_packet_new();

		irmo_packet_writei16(sendpacket, 
				     PACKET_FLAG_SYN|PACKET_FLAG_ACK);

		irmo_socket_sendpacket(packet->sock,
				       packet->src,
				       sendpacket);

		irmo_packet_free(sendpacket);
	}

	// dont do this if we're the server, or we'll get stuck in
	// an infinite loop :)
}

// run SYN FIN (disconnect)

static void socket_run_synfin(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoPacket *sendpacket;
	
	// connection refused?
	
	if (client->state == CLIENT_CONNECTING) {
		char *message;

		// read the reason message

		message = irmo_packet_readstring(packet);

		if (message)
			irmo_connection_error(client, "connection refused (%s)", message);
		else
			irmo_connection_error(client, "connection refused");

		client->state = CLIENT_DISCONNECTED;
	}

	// disconnect

	if (client->state == CLIENT_CONNECTED) {
		client->state = CLIENT_DISCONNECTED;
		client->connect_time = time(NULL);
		client->disconnect_wait = 1;

		irmo_client_callback_raise(client->disconnect_callbacks,
					   client);
	}

	if (client->state == CLIENT_DISCONNECTED) {
		
		// send a syn/fin/ack to reply

		sendpacket = irmo_packet_new();
		irmo_packet_writei16(sendpacket,
				     PACKET_FLAG_SYN | PACKET_FLAG_FIN
						     | PACKET_FLAG_ACK);

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       sendpacket);

		irmo_packet_free(sendpacket);
	}
}

static void socket_run_synfinack(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	
	if (client->state == CLIENT_DISCONNECTING) {
		client->state = CLIENT_DISCONNECTED;
		irmo_client_callback_raise(client->disconnect_callbacks,
					   client);
	}
}

static void socket_run_packet(IrmoPacket *packet)
{
	unsigned int flags;
	IrmoClient *client;

	// find a client from the socket hashtable

	packet->client = client
		= irmo_hash_table_lookup(packet->sock->server->clients, 
				      packet->src);

	// read packet header
	
	if (!irmo_packet_readi16(packet, &flags)) {
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
	
	irmo_proto_parse_packet(packet);
}

static int socket_run_client(void *key, IrmoClient *client,
				  void *user_data)
{
	irmo_client_run(client);

	// dont remove clients which arent disconnected

	if (client->state != CLIENT_DISCONNECTED)
		return 0;

	// if this is a client remotely disconnecting,
	// we wait a while before destroying the object
	
	if (client->disconnect_wait
	    && time(NULL) - client->connect_time < 10)
		return 0;
	
	irmo_client_internal_unref(client);

	// remove from socket list: return 1
	
	return 1;
}

void irmo_socket_run(IrmoSocket *sock)
{
	uint8_t buf[PACKET_BUFFER_LEN];
	struct sockaddr *addr;
	int addr_len;

	irmo_return_if_fail(sock != NULL);
	
	addr_len = irmo_sockaddr_len(socket_type_to_domain(sock->domain));
	addr = malloc(addr_len);
	
	while (1) {
		IrmoPacket packet;
		int result;
		unsigned int tmp_addr_len = addr_len;
		
		result = recvfrom(sock->sock,
				  buf,
				  PACKET_BUFFER_LEN,
				  0,
				  addr,
				  &tmp_addr_len);

		if (result < 0) {
			if (errno != EAGAIN)
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
	irmo_hash_table_foreach_remove(sock->server->clients,
				    (IrmoHashTableRemoveIterator) socket_run_client, NULL);
}

void irmo_socket_block_set(IrmoSocket **sockets, int nsockets, int timeout)
{
	fd_set set;
	struct timeval tv_timeout;
	int result;
	int max;
	int i;
	
	irmo_return_if_fail(sockets != NULL);
	irmo_return_if_fail(nsockets >= 0);
	
	FD_ZERO(&set);

	for (max=0,i=0; i<nsockets; ++i) {
		FD_SET(sockets[i]->sock, &set);
		if (max < sockets[i]->sock)
			max = sockets[i]->sock;
	}

	if (timeout > 0) {
		tv_timeout.tv_sec = timeout / 1000;
		tv_timeout.tv_usec = timeout % 1000;

		result = select(max+1, &set, NULL, NULL, &tv_timeout);
	} else {
		result = select(max+1, &set, NULL, NULL, NULL);
	}
	
	irmo_return_if_fail(result >= 0);
}

void irmo_socket_block(IrmoSocket *socket, int timeout)
{
	irmo_return_if_fail(socket != NULL);

	irmo_socket_block_set(&socket, 1, timeout);
}

