#ifndef IRMO_INTERNAL_SERVER_H
#define IRMO_INTERNAL_SERVER_H

#include "public/server.h"

#include "if_spec.h"
#include "client.h"
#include "socket.h"
#include "universe.h"

struct _IrmoServer {

	// reference count
	
	int refcount;
	
	// vhost hostname

	gchar *hostname;

	// socket this server is using

	IrmoSocket *socket;
	
	// universe being served by this server
	
	IrmoUniverse *universe;

	// spec for client universes

	IrmoInterfaceSpec *client_spec;
	
	// connected IrmoClients hashed by IP
	
	GHashTable *clients;
};

#endif /* #ifndef IRMO_INTERNAL_SERVER_H */

// $Log: not supported by cvs2svn $
// Revision 1.5  2003/02/23 00:00:04  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.4  2003/02/16 23:41:27  sdh300
// Reference counting for client and server objects
//
// Revision 1.3  2002/11/26 15:46:41  sdh300
// Fix compile and possible namespace conflicts with the "socket" function
//
// Revision 1.2  2002/11/26 15:43:05  sdh300
// Initial IrmoServer code
//
// Revision 1.1  2002/11/26 15:23:49  sdh300
// Initial network code
//

