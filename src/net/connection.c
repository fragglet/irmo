//
// Copyright (C) 2002-2008 Simon Howard
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
#include "base/assert.h"
#include "base/error.h"

#include "netbase/net-socket.h"

#include "connection.h"

IrmoConnection *irmo_connect(IrmoNetModule *net_module,
                             char *location,
                             unsigned int port,
                             IrmoInterface *iface, 
                             IrmoWorld *local_world)
{
	IrmoNetSocket *sock;
        IrmoNetAddress *addr;
	IrmoServer *server;
	IrmoClient *client;

	irmo_return_val_if_fail(net_module != NULL, NULL);
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

	return client;
} 

void irmo_disconnect(IrmoConnection *conn)
{
	irmo_return_if_fail(conn != NULL);

	// keep a watch on the client and stop it being destroyed

	irmo_client_ref(conn);

	// set disconnect

	irmo_client_disconnect(conn);
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
        irmo_return_if_fail(conn != NULL);

        irmo_server_block(conn->server, ms);
}

IrmoClientID irmo_connection_get_id(IrmoConnection *conn)
{
        irmo_return_val_if_fail(conn != NULL, 0);
        irmo_return_val_if_fail(conn->state != IRMO_CLIENT_CONNECTING, 0);

        return conn->server->remote_client_id;
}

IrmoClientState irmo_connection_get_state(IrmoConnection *conn)
{
        return irmo_client_get_state(conn);
}

