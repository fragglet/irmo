#include "connection.h"
#include "netlib.h"
#include "socket.h"

IrmoConnection *irmo_connect(int domain, gchar *location, int port,
			     IrmoInterfaceSpec *spec, 
			     IrmoUniverse *local_universe)
{
	IrmoConnection *connection;
	IrmoSocket *sock;
	struct sockaddr *addr;
	IrmoServer *server;
	IrmoClient *client;

	// create a socket
	
	sock = irmo_socket_new_unbound(domain);
	
	if (!sock)
		return NULL;

	// try to resolve the name

	addr = sockaddr_for_name(domain, location, port);
	
	// create a server for our local universe. for accessing the
	// local universe the server is seen as a client connecting
	// to our own local server (symmetrical)
	
	server = irmo_server_new(sock, NULL, local_universe, spec);

	// create a client object, also representing the servers
	// connection to us

	client = irmo_client_new(server, addr);
	irmo_client_ref(client);
	
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
		// connection failed
		// unreference objects we were using
		
		irmo_client_unref(client);
		irmo_server_unref(server);
		irmo_socket_unref(sock);
		
		return NULL;
	}

	// put everything inside a connection object

	connection = g_new0(IrmoConnection, 1);
	connection->sock = sock;
	connection->local_client = client;
	connection->local_server = server;
	connection->local_universe = local_universe;
	connection->universe = client->universe;

	if (local_universe)
		irmo_universe_ref(local_universe);

	return connection;
} 

IrmoSocket *irmo_connection_get_socket(IrmoConnection *conn)
{
	return conn->sock;
}

void irmo_connection_run(IrmoConnection *conn)
{
	irmo_socket_run(conn->sock);
}

IrmoUniverse *irmo_connection_get_universe(IrmoConnection *conn)
{
	return conn->universe;
}

// $Log: not supported by cvs2svn $
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
