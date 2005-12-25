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

	message = g_strdup_printf(s, args);

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
	GTimeVal tv_timeout;
	struct timeval tv_timeout2;
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
		irmo_timeval_from_ms(timeout, &tv_timeout);
		tv_timeout2.tv_sec = tv_timeout.tv_sec;
		tv_timeout2.tv_usec = tv_timeout.tv_usec;
		result = select(max+1, &set, NULL, NULL, &tv_timeout2);
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

// $Log$
// Revision 1.29  2005/12/25 00:48:29  fraggle
// Use internal memory functions, rather than the glib ones
//
// Revision 1.28  2005/12/25 00:38:18  fraggle
// Use internal macros instead of glib ones for assertation checks
//
// Revision 1.27  2005/12/24 00:15:59  fraggle
// Use the C99 "uintN_t" standard integer types rather than the glib
// guint types.
//
// Revision 1.26  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.25  2004/04/17 22:19:57  fraggle
// Use glib memory management functions where possible
//
// Revision 1.24  2004/04/17 22:02:35  fraggle
// Fix crash on disconnect from multiple delete
//
// Revision 1.23  2004/01/06 01:36:18  fraggle
// Remove vhosting. Simplify the server API.
//
// Revision 1.22  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.21  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.20  2003/11/20 00:19:17  fraggle
// Add some fixes to get compiling under windows
//
// Revision 1.19  2003/11/18 19:32:19  fraggle
// Use GTimeVal instead of struct timeval
//
// Revision 1.18  2003/11/18 19:24:22  fraggle
// Fix compile errors and warnings introduced by previous commit
//
// Revision 1.17  2003/11/18 18:14:47  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.16  2003/11/05 02:05:41  fraggle
// Use guint8 instead of unsigned char
//
// Revision 1.15  2003/10/18 01:34:45  fraggle
// Better error reporting for connecting, allow server to send back an
// error message when refusing connections
//
// Revision 1.14  2003/10/17 23:33:05  fraggle
// protocol version checking
//
// Revision 1.13  2003/10/14 22:12:50  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.12  2003/10/14 00:53:43  fraggle
// Remove pointless inlinings
//
// Revision 1.11  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.10  2003/09/01 18:59:27  fraggle
// Add a timeout parameter for blocking on sockets. Use block function
// internally.
//
// Revision 1.9  2003/09/01 18:52:51  fraggle
// Blocking functions for IrmoSocket
//
// Revision 1.8  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.7  2003/09/01 01:25:49  fraggle
// Improve packet code; increase packet size exponentially.
// Remove the need to specify the size when creating a new packet object.
//
// Revision 1.6  2003/08/30 03:08:00  fraggle
// Use irmo_client_internal_unref instead of unreffing externally. Make
// irmo_client_destroy static now.
//
// Revision 1.5  2003/08/26 16:15:40  fraggle
// fix bug with reconnect immediately after a disconnect
//
// Revision 1.4  2003/08/26 14:57:31  fraggle
// Remove AF_* BSD sockets dependency from Irmo API
//
// Revision 1.3  2003/08/18 01:23:14  fraggle
// Use G_INLINE_FUNC instead of inline for portable inline function support
//
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
