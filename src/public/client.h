// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

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

/*! 
 * \brief Callback function for client actions.
 *
 * Functions of this type are used for client disconnect callbacks.
 */

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
 *
 * \return          A \ref IrmoCallback object representing the watch.
 */

IrmoCallback *irmo_client_watch_disconnect(IrmoClient *client,
					   IrmoClientCallback func, 
					   gpointer user_data);

/*!
 * \brief Get the ping time for a client
 *
 * Returns the round trip time (ping time) for a particular client.
 * The accuracy of this is reliant on the amount of data which has
 * been sent to the client. If no or very little data has been sent
 * recently it may be inaccurate.
 *
 * \return Round Trip Time in milliseconds
 *
 */

int irmo_client_ping_time(IrmoClient *client);

/*!
 * \brief Set the maximum send window size
 *
 * This sets the maximum send window size for network transfer. This
 * is dependent on the bandwidth of the network connection. If the
 * maximum send window size is not set manually, the library will
 * attempt to determine an upper bound using congestion avoidance
 * algorithms, which may produce poor performance.
 *
 * \param client   The client to set the value on
 * \param max      The maximum send window size (in bytes)
 */

void irmo_client_set_max_sendwindow(IrmoClient *client, int max);

/*!
 * \brief Get the address of the client.
 *
 * This returns the BSD sockets address object of the remote
 * client.
 * 
 * \param client  The client to query.
 *
 */

struct sockaddr *irmo_client_get_addr(IrmoClient *client);

//! \}

#endif /* #ifndef IRMO_CLIENT_H */

// $Log$
// Revision 1.2  2003/07/22 02:05:40  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.10  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.9  2003/05/07 15:47:18  sdh300
// Add irmo_client_get_addr function to get client's remote address
// Add missing glib assertations
//
// Revision 1.8  2003/05/04 00:28:17  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.7  2003/04/21 20:10:22  sdh300
// Add a function to the API to get the RTT for a client
//
// Revision 1.6  2003/03/17 17:35:15  sdh300
// Add disconnect callbacks for clients
// Move IrmoClientCallback to client.h
//
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

