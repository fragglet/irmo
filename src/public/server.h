#ifndef IRMO_SERVER_H
#define IRMO_SERVER_H

typedef struct _IrmoServer IrmoServer;

#include "if_spec.h"
#include "client.h"
#include "socket.h"
#include "universe.h"

/*!
 * \brief Create a new server
 *
 * Create a new \ref IrmoServer listening for connections on a
 * particular vhost.
 *
 * \param sock      The socket to listen on.
 * \param hostname  The vhost hostname to listen on. If NULL is passed
 *                  for this parameter, this server will be the default
 *                  server for the socket. If there is an existing
 *                  default server, the function will fail.
 * \param universe  The universe the server will serve.
 * \param spec      An interface specification describing the interface
 *                  for the client universe of the connecting clients.
 * \return          A new IrmoServer or NULL if the function fails.
 */

IrmoServer *server_new(IrmoSocket *sock, gchar *hostname,
		       IrmoUniverse *universe, InterfaceSpec *spec);

void server_ref(IrmoServer *server);

void server_unref(IrmoServer *server);

#endif /* #ifndef IRMO_SERVER_H */

// $Log: not supported by cvs2svn $
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
