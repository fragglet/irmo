#include "connection.h"
#include "netlib.h"
#include "socket.h"

IrmoConnection *irmo_connect(int domain, gchar *location, int port,
			     InterfaceSpec *spec, IrmoUniverse *local_universe)
{
	IrmoConnection *connection;
	IrmoSocket *sock = _socket_new_unbound(domain);
	struct sockaddr *addr;
	IrmoServer *server;
	IrmoClient *client;
	
	if (!sock)
		return NULL;

	// try to resolve the name

	addr = sockaddr_for_name(domain, location, port);
	
	// create a server for our local universe. for accessing the
	// local universe the server is seen as a client connecting
	// to our own local server (symmetrical)
	
	server = server_new(sock, NULL, local_universe, spec);

	// create a client object, also representing the servers
	// connection to us

	client = _client_new(server, addr);

	// now initiate the connection
	// send SYN packets to the server once every second and
	// wait for replies.
	// if client->state is set to CLIENT_CONNECTED
	// the server responded with a SYN ACK
	// if client->state is set to CLIENT_DISCONNECTED,
	// the server responded with a SYN FIN and something went wrong

	
	
	// put everything inside a connection object

	connection = g_new0(IrmoConnection, 1);
	connection->sock = sock;
	connection->local_client = client;
	connection->local_server = server;
	connection->local_universe = local_universe;

	if (local_universe)
		universe_ref(local_universe);

	return connection;
} 

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/06 02:10:11  sdh300
// Add missing CVS tags
//
