#ifndef IRMO_INTERNAL_CLIENT_H
#define IRMO_INTERNAL_CLIENT_H

#include "public/client.h"

#include <netinet/in.h>

#include "sendatom.h"
#include "server.h"
#include "socket.h"
#include "universe.h"

// maximum sendwindow size

#define MAX_SENDWINDOW 1024

typedef enum {
	CLIENT_CONNECTING,          // received first syn, sent syn ack
	CLIENT_CONNECTED,           // received syn ack reply, normal op
	CLIENT_DISCONNECTED,        // killed with syn fin
	CLIENT_DISCONNECTING,       // waiting for ack of disconnect request 
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

	// when a client remotely disconnects, keep the client object
	// for several seconds before destroying it (if they do not
	// receive the disconnect ack they may send another disconnect
	// request, in which case another ack will need to be sent)
	
	gboolean disconnect_wait;

	// send queue 
	
	GQueue *sendq;

	// change entries in sendq are hashed by object id so
	// new changes can be added to the existing sendatoms
	
	GHashTable *sendq_hashtable;

	// position of start of send window in stream

	int sendwindow_start;
	
	// send window

	IrmoSendAtom *sendwindow[MAX_SENDWINDOW];
	int sendwindow_size;

	// receive window

	int recvwindow_start;

	IrmoSendAtom **recvwindow;
	int recvwindow_size;

	// if true, we need to send an ack to the client to acknowledge
	// something it has sent us
	
	gboolean need_ack;
};

// create a new client, attached to a particular server

IrmoClient *irmo_client_new(IrmoServer *server, struct sockaddr *addr);

// 'run' a client, called by irmo_socket_run

void irmo_client_run(IrmoClient *client);

// destroy client data structure

void irmo_client_destroy(IrmoClient *client);

// run through sendatoms waiting in the receive window

void irmo_client_run_recvwindow(IrmoClient *client);

#endif /* #ifndef IRMO_INTERNAL_CLIENT_H */

// $Log: not supported by cvs2svn $
// Revision 1.15  2003/03/07 12:32:15  sdh300
// Add missing prototype and some documentation
//
// Revision 1.14  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.13  2003/03/06 20:15:24  sdh300
// Initial ack code
//
// Revision 1.12  2003/03/05 15:28:13  sdh300
// Add receive window and extra data for sendatoms in the receive window.
//
// Revision 1.11  2003/03/03 21:03:06  sdh300
// Initial packet building code
//
// Revision 1.10  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.9  2003/02/23 00:00:03  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
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

