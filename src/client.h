#ifndef IRMO_INTERNAL_CLIENT_H
#define IRMO_INTERNAL_CLIENT_H

#include "public/client.h"

#include <netinet/in.h>

#include "server.h"
#include "socket.h"
#include "universe.h"

typedef enum {
	CLIENT_CONNECTING,             /* received first syn, sent syn ack */
	CLIENT_CONNECTED,              /* received syn ack reply, normal op */
	CLIENT_DISCONNECTED,           /* killed with syn fin */
} IrmoClientState;

// client

struct _IrmoClient {

	int refcount;
	
	IrmoClientState state;
	
	// server client is connected to

	IrmoServer *server;
	
	// clients remote universe
	
	IrmoUniverse *universe;

	// address:

	struct sockaddr *addr;

	// protocol stuff (internal)

	// time last syn/synack was sent
	time_t _connect_time;
	gint _connect_attempts;

	// send queue 
	
	GQueue *sendq;

	// change entries in sendq are hashed by object id so
	// new changes can be added to the existing sendatoms
	
	GHashTable *sendq_hashtable;
};

IrmoClient *_client_new(IrmoServer *server, struct sockaddr *addr);
void _client_run(IrmoClient *client);
void _client_destroy(IrmoClient *client);

#endif /* #ifndef IRMO_INTERNAL_CLIENT_H */

// $Log: not supported by cvs2svn $
// Revision 1.8  2003/02/20 18:24:59  sdh300
// Use GQueue instead of a GPtrArray for the send queue
// Initial change/destroy code
//
// Revision 1.7  2003/02/18 20:26:41  sdh300
// Initial send queue building/notification code
//
// Revision 1.6  2003/02/18 18:25:40  sdh300
// Initial queue object code
//
// Revision 1.5  2003/02/16 23:41:26  sdh300
// Reference counting for client and server objects
//
// Revision 1.4  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.3  2003/02/06 02:41:25  sdh300
// Add CLIENT_DISCONNECTED for disconnected clients
//
// Revision 1.2  2003/02/03 20:57:22  sdh300
// Initial client code
//
// Revision 1.1  2002/11/26 15:23:49  sdh300
// Initial network code
//

