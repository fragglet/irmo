#ifndef IRMO_SOCKET_H
#define IRMO_SOCKET_H

/*!
 * \addtogroup socket
 * \{
 */


/*!
 * \brief an IrmoSocket object.
 */

typedef struct _IrmoSocket IrmoSocket;

#include <netinet/in.h>

#include "client.h"
#include "server.h"

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

/*!
 * \brief Add a reference to a socket
 *
 * \param sock  Socket to reference
 * \sa socket_unref
 *
 */

void socket_ref(IrmoSocket *sock);

/*!
 * \brief Remove a reference to a socket
 *
 * The number of references to a socket is counted. The count starts at
 * 1. When the count reaches 0, the socket is destroyed.
 *
 * \param sock  Socket to unreference
 * \sa socket_ref
 */

void socket_unref(IrmoSocket *sock);

/*!
 * \brief Run socket
 *
 * This function must be called periodically to check for new packets
 * received by the socket and send new packets required by the network
 * protocol.
 *
 * \param sock   The socket to update
 */

void socket_run(IrmoSocket *sock);

//! \}

#endif /* #ifndef IRMO_SOCKET_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/02/23 00:00:07  sdh300
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
