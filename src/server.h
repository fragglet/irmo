#ifndef IRMO_SERVER_H
#define IRMO_SERVER_H

typedef struct _IrmoServer IrmoServer;

#include "if_spec.h"
#include "client.h"
#include "socket.h"
#include "universe.h"

struct _IrmoServer {

	// vhost hostname

	gchar *hostname;

	// socket this server is using

	IrmoSocket *socket;
	
	// universe being served by this server
	
	IrmoUniverse *universe;

	// connected IrmoClients hashed by IP
	
	GHashTable *clients;
};

/*!
 * \brief Create a new server
 *
 * Create a new \ref IrmoServer listening for connections on a
 * particular vhost.
 *
 * \param socket    The socket to listen on.
 * \param hostname  The vhost hostname to listen on. If NULL is passed
 *                  for this parameter, this server will be the default
 *                  server for the socket. If there is an existing
 *                  default server, the function will fail.
 * \param universe  The universe the server will serve.
 * \param spec      An interface specification describing the interface
 *                  for the client universe of the connecting clients.
 * \return          A new IrmoServer or NULL if the function fails.
 */

IrmoServer *server_new(IrmoSocket *socket, gchar *hostname,
		       IrmoUniverse *universe, InterfaceSpec *spec);


#endif /* #ifndef IRMO_SERVER_H */

// $Log: not supported by cvs2svn $

