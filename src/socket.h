#ifndef IRMO_INTERNAL_SOCKET_H
#define IRMO_INTERNAL_SOCKET_H

#include "public/socket.h"

#include <glib.h>
#include <netinet/in.h>

#include "client.h"
#include "server.h"

// fixme: packet.h is supposed to be internal and should not be here
// split off internal socket stuff to a seperate header
#include "packet.h"

typedef enum {
	SOCKET_SERVER,
	SOCKET_CLIENT,
} IrmoSocketType;

struct _IrmoSocket {

	IrmoSocketType type;
	
	// reference count
	
	int refcount;
	
	// domain/port and socket

	int domain;
	int port;
	int sock;

	// attached servers hashed by name

	GHashTable *servers;

	// the default server to use (if the hostname a client is trying to
	// connect to does not exist)

	IrmoServer *default_server;
	
	// connected clients hashed by IP

	GHashTable *clients;
};

IrmoSocket *irmo_socket_new_unbound(int domain);
void irmo_socket_sendpacket(IrmoSocket *sock, struct sockaddr *dest,
			    IrmoPacket *packet);

#endif /* #ifndef IRMO_INTERNAL_SOCKET_H */

// $Log: not supported by cvs2svn $
// Revision 1.8  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.7  2003/02/23 00:00:04  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.6  2003/02/11 19:18:43  sdh300
// Initial working connection code!
//
// Revision 1.5  2003/02/11 17:53:01  sdh300
// Add socket_run and documentation (missing from the header)
//
// Revision 1.4  2003/02/06 00:59:26  sdh300
// Split up socket constructor function, so that unbound sockets can
// be created for clients
//
// Revision 1.3  2002/12/02 21:32:51  sdh300
// reference counting for IrmoSockets
//
// Revision 1.2  2002/11/26 15:32:12  sdh300
// store port number in IrmoSocket objects as well
//
// Revision 1.1  2002/11/26 15:23:50  sdh300
// Initial network code
//
