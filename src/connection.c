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

#include "connection.h"
#include "netlib.h"
#include "socket.h"

IrmoConnection *irmo_connect(IrmoSocketDomain domain, 
			     char *location, int port,
			     IrmoInterfaceSpec *spec, 
			     IrmoWorld *local_world)
{
	IrmoSocket *sock;
	struct sockaddr *addr;
	IrmoServer *server;
	IrmoClient *client;

	g_return_val_if_fail(location != NULL, NULL);

	if (domain == IRMO_SOCKET_AUTO) {
		IrmoConnection *conn;

#ifdef USE_IPV6
		// try IPv6
		
		conn = irmo_connect(IRMO_SOCKET_IPV6, location, port, spec,
				    local_world);

		if (conn)
			return conn;
#endif
		// fall back to v4

		return irmo_connect(IRMO_SOCKET_IPV4, location, port, spec,
				    local_world);
	}
	
	// try to resolve the name

	addr = irmo_sockaddr_for_name(domain, location, port);

	if (!addr)
		return NULL;
	
	// create a socket
	
	sock = irmo_socket_new_unbound(domain);
	
	if (!sock)
		return NULL;

	// create a server for our local world. for accessing the
	// local world the server is seen as a client connecting
	// to our own local server (symmetrical)
	
	server = irmo_server_new(sock, NULL, local_world, spec);

	// only the server is using this socket 
	
	irmo_socket_unref(sock);
	
	// create a client object, also representing the servers
	// connection to us

	client = irmo_client_new(server, addr);

	// reference is on the client, which implies the server
	
	irmo_client_ref(client);
	irmo_server_unref(server);
	
	// now initiate the connection
	// send SYN packets to the server once every second and
	// wait for replies.
	// if client->state is set to CLIENT_CONNECTED
	// the server responded with a SYN ACK
	// if client->state is set to CLIENT_DISCONNECTED,
	// the server responded with a SYN FIN and something went wrong

	while (client->state == CLIENT_CONNECTING) {
		irmo_socket_run(sock);
	}

	if (client->state == CLIENT_DISCONNECTED) {

		irmo_error_report("irmo_connect", "%s", 
				  client->connection_error);
		
		// connection failed
		// delete client object
		
		irmo_client_unref(client);

		return NULL;
	}

	return client;
} 

void irmo_disconnect(IrmoConnection *conn)
{
	g_return_if_fail(conn != NULL);
	
	// keep a watch on the client and stop it being destroyed
	
	irmo_client_ref(conn);

	// set disconnect
	
	irmo_client_disconnect(conn);
	
	// loop until we disconnect from the server (either from
	// getting a positive disconnect reply or from timeout)

	while (conn->state != CLIENT_DISCONNECTED) {
		irmo_socket_run(conn->server->socket);
		irmo_socket_block(conn->server->socket, 100);
	}

	irmo_client_unref(conn);
}

IrmoSocket *irmo_connection_get_socket(IrmoConnection *conn)
{
	g_return_val_if_fail(conn != NULL, NULL);
	
	return conn->server->socket;
}

void irmo_connection_run(IrmoConnection *conn)
{
	g_return_if_fail(conn != NULL);
	
	irmo_socket_run(conn->server->socket);
}

IrmoWorld *irmo_connection_get_world(IrmoConnection *conn)
{
	g_return_val_if_fail(conn != NULL, NULL);
	
	return conn->world;
}

void irmo_connection_ref(IrmoConnection *conn)
{
	g_return_if_fail(conn != NULL);
	
	irmo_client_ref(conn);
}

void irmo_connection_unref(IrmoConnection *conn)
{
	g_return_if_fail(conn != NULL);

	irmo_client_unref(conn);
}

void irmo_connection_error(IrmoConnection *conn, char *s, ...)
{
	va_list args;

	if (conn->connection_error)
		free(conn->connection_error);

	va_start(args, s);

	conn->connection_error = g_strdup_vprintf(s, args);

	va_end(args);
}

// $Log$
// Revision 1.11  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.10  2003/10/18 01:34:45  fraggle
// Better error reporting for connecting, allow server to send back an
// error message when refusing connections
//
// Revision 1.9  2003/09/13 15:21:31  fraggle
// Make sure we resolve a name properly before trying to connect
//
// Revision 1.8  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.7  2003/09/01 18:59:27  fraggle
// Add a timeout parameter for blocking on sockets. Use block function
// internally.
//
// Revision 1.6  2003/09/01 17:02:58  fraggle
// Turn irmo_connection_[un]ref into real functions
//
// Revision 1.5  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.4  2003/08/26 14:57:31  fraggle
// Remove AF_* BSD sockets dependency from Irmo API
//
// Revision 1.3  2003/08/16 14:06:22  fraggle
// Report an error when failing to connect
//
// Revision 1.2  2003/08/06 16:15:18  fraggle
// IPv6 support
//
// Revision 1.1.1.1  2003/06/09 21:33:23  fraggle
// Initial sourceforge import
//
// Revision 1.15  2003/06/09 21:06:50  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.14  2003/04/25 01:26:18  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.13  2003/03/17 16:48:23  sdh300
// Add ability to disconnect from servers and to disconnect clients
//
// Revision 1.12  2003/03/17 15:45:34  sdh300
// Remove 'IrmoConnection' object; make it into a typedef for IrmoClient
//
// Revision 1.11  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.10  2003/03/06 19:33:50  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.9  2003/03/06 19:21:39  sdh300
// Fill in missing field in connection objects
//
// Revision 1.8  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.7  2003/02/23 00:45:39  sdh300
// Add universe access functions for client, connection
//
// Revision 1.6  2003/02/23 00:26:46  sdh300
// Add some documentation and helper functions for IrmoConnection
//
// Revision 1.5  2003/02/18 17:41:37  sdh300
// Add timeout for connect (6 attempts)
//
// Revision 1.4  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.3  2003/02/06 02:39:04  sdh300
// Add missing netlib.h, fix call to sockaddr_for_name
//
// Revision 1.2  2003/02/06 02:10:11  sdh300
// Add missing CVS tags
//
