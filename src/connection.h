#ifndef IRMO_INTERNAL_CONNECTION_H
#define IRMO_INTERNAL_CONNECTION_H

#include "public/connection.h"

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

#endif /* #ifndef IRMO_INTERNAL_CONNECTION_H */

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/06 02:10:11  sdh300
// Add missing CVS tags
//
