#ifndef IRMO_CONNECTION_H
#define IRMO_CONNECTION_H

typedef struct _IrmoConnection IrmoConnection;

#include "if_spec.h"
#include "server.h"
#include "universe.h"

struct _IrmoConnection {

	// socket
	
	IrmoSocket *sock;

	// the server object is "serving" the local
	// universe to the server (symmetrical)

	IrmoServer *local_server;

	// client object. this is the server's "connection" to the
	// local universe we are serving to it.

	IrmoClient *local_client;
	
	// local universe that we are serving to the server
	
	IrmoUniverse *local_universe;

	// the universe on the remote server
	
	IrmoUniverse *universe;
};

IrmoConnection *irmo_connect(int domain, gchar *location, int port,
                             InterfaceSpec *spec,
			     IrmoUniverse *local_universe);


#endif /* #ifndef IRMO_CONNECTION_H */
