#ifndef IRMO_CLIENT_H
#define IRMO_CLIENT_H

#include <glib.h>

/*!
 * \addtogroup client
 * \{
 */

/*!
 * \brief An IrmoClient object
 *
 * This represents a client connected to a \ref IrmoServer server.
 */

typedef struct _IrmoClient IrmoClient;

typedef void (*IrmoClientCallback) (IrmoClient *client, gpointer user_data);

#include "server.h"
#include "socket.h"
#include "universe.h"

/*!
 * \brief Add a reference to a client
 *
 * When clients are disconnected from the server their client objects
 * get automatically deleted after a while. You can add a reference
 * to a IrmoClient object to notify that you are using the object and
 * it will not be deleted.
 *
 * \param client  The client object to reference
 * \sa irmo_client_unref
 */

void irmo_client_ref(IrmoClient *client);

/*!
 * \brief Unreference a client object 
 *
 * Specify that you are no longer referencing a client object
 *
 * \param client   The client object to unreference
 * \sa irmo_client_ref
 */

void irmo_client_unref(IrmoClient *client);

/*!
 * \brief Forcibly disconnect a client
 *
 * \param client      The client to disconnect
 *
 */

void irmo_client_disconnect(IrmoClient *client);

/*!
 * \brief Get the clients universe object
 *
 * While a server can serve a \ref IrmoUniverse object to connecting 
 * clients, the clients can also serve a universe object back to the
 * server. This function returns a reference to the local copy of 
 * that remote universe object if it exists, or NULL if it does not.
 *
 * \sa irmo_connect
 * \sa irmo_connection_get_universe
 *
 */

IrmoUniverse *irmo_client_get_universe(IrmoClient *client);

/*!
 * \brief Watch for client disconnection
 *
 * Set a watch on a client. When that client disconnects, a
 * \ref IrmoClientCallback callback function is invoked.
 *
 * \param client    The client to watch
 * \param func      The callback function to invoke
 * \param user_data Extra data to pass to the callback function
 * \sa irmo_client_unwatch_disconnect
 */

void irmo_client_watch_disconnect(IrmoClient *client,
				  IrmoClientCallback func, gpointer user_data);

/*!
 * \brief Unset a watch set with \ref irmo_client_watch_disconnect
 *
 * Unset a disconnect watch. All parameters must be the same as those
 * used to initially set the watch.
 */

void irmo_client_unwatch_disconnect(IrmoClient *client,
				    IrmoClientCallback func,
				    gpointer user_data);

//! \}

#endif /* #ifndef IRMO_CLIENT_H */

// $Log: not supported by cvs2svn $
// Revision 1.5  2003/03/07 12:17:21  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.4  2003/03/07 10:48:07  sdh300
// Add new sections to documentation
//
// Revision 1.3  2003/02/23 00:45:43  sdh300
// Add universe access functions for client, connection
//
// Revision 1.2  2003/02/23 00:35:16  sdh300
// Add some missing documentation
//
// Revision 1.1  2003/02/23 00:00:06  sdh300
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

