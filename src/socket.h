#ifndef IRMO_SOCKET_H
#define IRMO_SOCKET_H

typedef struct _IrmoSocket IrmoSocket;

#include <glib.h>

#include "client.h"
#include "server.h"

struct _IrmoSocket {

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

/*!
 * \brief Create a new socket
 *
 * Create a new socket listening on a particular port. Servers can then
 * be attached to the socket to serve universes on different vhosts.
 *
 * \param domain  Domain to use. Normally this will be AF_INET.
 * \param port    The port number to listen on
 * \return        A new IrmoSocket or NULL if the new socket could
 *                not be created.
 */

IrmoSocket *socket_new(int domain, int port);

#endif /* #ifndef IRMO_SOCKET_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2002/11/26 15:23:50  sdh300
// Initial network code
//
