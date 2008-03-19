//
// Copyright (C) 2002-3 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "netbase/net-socket.h"

#include "connection.h"

IrmoConnection *irmo_connect(IrmoNetModule *net_module,
			     char *location, int port,
			     IrmoInterface *iface, 
			     IrmoWorld *local_world)
{
	IrmoNetSocket *sock;
        IrmoNetAddress *addr;
	IrmoServer *server;
	IrmoClient *client;

	irmo_return_val_if_fail(location != NULL, NULL);

	// try to resolve the name

	addr = irmo_net_address_resolve(net_module, location, port);

	if (addr == NULL) {
		return NULL;
        }
	
	// create a socket
	
	sock = irmo_net_socket_open_unbound(net_module);
	
	if (sock == NULL) {
		return NULL;
        }

	// create a server for our local world. for accessing the
	// local world the server is seen as a client connecting
	// to our own local server (symmetrical)
	
	server = irmo_server_new_from(sock, local_world, iface);
        server->internal_server = 1;

	// create a client object, also representing the server's
	// connection to us

	client = irmo_client_new(server, addr);
        irmo_net_address_unref(addr);

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
		irmo_server_run(server);
	}

	if (client->state == CLIENT_DISCONNECTED) {

		irmo_error_report("irmo_connect", "%s", 
				  client->connection_error);
		
		// connection failed
		// delete client object, shutdown socket, etc.
		
		irmo_client_unref(client);

		return NULL;
	}

	return client;
} 

void irmo_disconnect(IrmoConnection *conn)
{
	irmo_return_if_fail(conn != NULL);
	
	// keep a watch on the client and stop it being destroyed
	
	irmo_client_ref(conn);

	// set disconnect
	
	irmo_client_disconnect(conn);
	
	// loop until we disconnect from the server (either from
	// getting a positive disconnect reply or from timeout)

	while (conn->state != CLIENT_DISCONNECTED) {
		irmo_server_run(conn->server);
		irmo_net_socket_block(conn->server->socket, 100);
	}

	irmo_client_unref(conn);
}

void irmo_connection_run(IrmoConnection *conn)
{
	irmo_return_if_fail(conn != NULL);
	
	irmo_server_run(conn->server);
}

IrmoWorld *irmo_connection_get_world(IrmoConnection *conn)
{
	irmo_return_val_if_fail(conn != NULL, NULL);
	
	return conn->world;
}

void irmo_connection_ref(IrmoConnection *conn)
{
	irmo_return_if_fail(conn != NULL);
	
	irmo_client_ref(conn);
}

void irmo_connection_unref(IrmoConnection *conn)
{
	irmo_return_if_fail(conn != NULL);

	irmo_client_unref(conn);
}

void irmo_connection_error(IrmoConnection *conn, char *s, ...)
{
	va_list args;

	if (conn->connection_error != NULL) {
		free(conn->connection_error);
        }

	va_start(args, s);

	conn->connection_error = irmo_vasprintf(s, args);

	va_end(args);
}

void irmo_connection_block(IrmoConnection *conn, int ms)
{
        irmo_server_block(conn->server, ms);
}

