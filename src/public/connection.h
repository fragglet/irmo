#ifndef IRMO_CONNECTION_H
#define IRMO_CONNECTION_H

/*!
 * \addtogroup connection
 * \{
 */

/*!
 * \brief An IrmoConnection object 
 *
 * This represents a connection to a \ref IrmoServer server running
 * on a remote machine
 */

typedef struct _IrmoConnection IrmoConnection;

#include "if_spec.h"
#include "server.h"
#include "universe.h"

/*!
 * \brief Establish a new connection to a server
 *
 * This establishes a new connection to a server on a remote machine.
 * If successful, an IrmoConnection object is returned representing
 * the connection to the server.
 *
 * Care must be taken in the interface specification and the local 
 * universe: their types must match those expected by the server.
 * (see \ref irmo_server_new)
 *
 * \param domain         Domain of the socket to use. Usually this will
 *                       be AF_INET.
 * \param location       The hostname of the remote machine to connect to.
 * \param port           The port on the remote machine on which the server
 *                       is running.
 * \param spec           An interface specification object describing the
 *                       remove universe served by the server, or NULL if
 *                       the server is not serving a universe.
 * \param local_universe A local universe to serve back to the server, or
 *                       NULL not to serve any universe back.
 * \return               A IrmoConnection object representing the
 *                       connection, or NULL if a connection could not be
 *                       established.
 */

IrmoConnection *irmo_connect(int domain, gchar *location, int port,
                             IrmoInterfaceSpec *spec,
			     IrmoUniverse *local_universe);

/*!
 * \brief Get the socket used by a IrmoConnection
 * 
 * Returns the socket object being used by a connection for network
 * communications.
 *
 */

IrmoSocket *irmo_connection_get_socket(IrmoConnection *conn);

/*!
 * \brief Check for new packets received from the server.
 *
 * This function must be called periodically to check for new packets
 * received socket and send new packets required by the network
 * protocol.
 * 
 * This is identical to:
 *   irmo_socket_run(irmo_connection_get_socket(conn));
 *
 * \sa irmo_socket_run
 */

void irmo_connection_run(IrmoConnection *conn);

/*!
 * \brief Get the universe object for the remote server
 *
 * Servers can serve a \ref IrmoUniverse object to their connecting
 * clients. This function returns a reference to the local copy of 
 * the remote universe (or NULL if the server is not serving a
 * universe)
 *
 * \sa irmo_server_new
 * \sa client_get_universe
 */

IrmoUniverse *irmo_connection_get_universe(IrmoConnection *conn);

//! \}

#endif /* #ifndef IRMO_CONNECTION_H */

// $Log: not supported by cvs2svn $
// Revision 1.5  2003/03/07 10:48:07  sdh300
// Add new sections to documentation
//
// Revision 1.4  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.3  2003/02/23 00:45:43  sdh300
// Add universe access functions for client, connection
//
// Revision 1.2  2003/02/23 00:26:57  sdh300
// Add some documentation and helper functions for IrmoConnection
//
// Revision 1.1  2003/02/23 00:00:06  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.2  2003/02/06 02:10:11  sdh300
// Add missing CVS tags
//
