#ifndef IRMO_SERVER_H
#define IRMO_SERVER_H

/*!
 * \addtogroup server
 * \{
 */

/*!
 * \brief An IrmoServer object
 */

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
		       IrmoUniverse *universe, IrmoInterfaceSpec *spec);

/*!
 * \brief Reference a server object
 *
 * Add a reference to a server object. When a server is created its 
 * reference count is set to 1. References can be added with server_ref
 * and removed with \ref server_unref. When the reference count reaches
 * 0 the server is automatically shut down and destroyed.
 *
 * \param server   The server object to reference.
 * \sa server_unref
 *
 */

void server_ref(IrmoServer *server);

/*!
 * \brief  Unreference a server object
 * 
 * \param server   The server object to unreference
 * \sa server_ref
 *
 */

void server_unref(IrmoServer *server);

//! \}

#endif /* #ifndef IRMO_SERVER_H */

// $Log: not supported by cvs2svn $
// Revision 1.3  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.2  2003/02/23 00:35:16  sdh300
// Add some missing documentation
//
// Revision 1.1  2003/02/23 00:00:07  sdh300
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

