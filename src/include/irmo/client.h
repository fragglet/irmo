//
// Copyright (C) 2002-2008 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#ifndef IRMO_CLIENT_H
#define IRMO_CLIENT_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * The @ref IrmoClient object represents clients currently 
 * connected to a server. For each client currently connected
 * there is an associated IrmoClient object.
 *
 * Once clients are disconnected from the server, they are
 * automatically 'garbage collected' and destroyed. However, 
 * the @ref irmo_client_ref and @ref irmo_client_unref functions can be
 * used to hold a reference to an IrmoClient object and prevent
 * it from being destroyed.
 *
 * @addtogroup client
 * \{
 */

/*!
 * Add a reference to a client.
 *
 * When clients are disconnected from the server their client objects
 * get automatically deleted after a while. You can add a reference
 * to a IrmoClient object to notify that you are using the object and
 * it will not be deleted.
 *
 * @param client  The client object to reference.
 * @sa irmo_client_unref
 */

void irmo_client_ref(IrmoClient *client);

/*!
 * Unreference a client object.
 *
 * Specify that you are no longer referencing a client object
 *
 * @param client   The client object to unreference.
 * @sa irmo_client_ref
 */

void irmo_client_unref(IrmoClient *client);

/*!
 * Forcibly disconnect a client.
 *
 * @param client      The client to disconnect.
 *
 */

void irmo_client_disconnect(IrmoClient *client);

/*!
 * Get a client's world object.
 *
 * While a server can serve a @ref IrmoWorld object to connecting 
 * clients, a client can also serve a world object back to the
 * server. 
 *
 * @param client   The client.
 * @return         A reference to the local copy of the remote world
 *                 object if it exists, or NULL if it does not.
 *
 * @sa irmo_connect
 * @sa irmo_connection_get_world
 *
 */

IrmoWorld *irmo_client_get_world(IrmoClient *client);

/*!
 * Get a client's ID number.
 *
 * On connection, each client is assigned a unique ID number.
 * This returns the ID assigned to the specified client.
 *
 * @param client   The client.
 * @return         The client's unique ID.
 */

IrmoClientID irmo_client_get_id(IrmoClient *client);

/*!
 * Get the connection status of a client.
 *
 * @param client   The client.
 * @return         The connection state of the client.
 */

IrmoClientState irmo_client_get_state(IrmoClient *client);

/*!
 * Watch for a client state change.
 *
 * Set a watch on a client. When the connection state of the client
 * changes to the specified state, a @ref IrmoClientCallback callback
 * function is invoked.
 *
 * @param client     The client to watch.
 * @param state      The state to watch for.
 * @param func       The callback function to invoke.
 * @param user_data  Extra data to pass to the callback function.
 *
 * @return           A @ref IrmoCallback object representing the watch.
 */

IrmoCallback *irmo_client_watch_state(IrmoClient *client,
                                      IrmoClientState state,
                                      IrmoClientCallback func,
                                      void *user_data);

/*!
 * Get the ping time for a client.
 *
 * Returns the round trip time (ping time) for a particular client.
 * The accuracy of this is reliant on the amount of data which has
 * been sent to the client. If no or very little data has been sent
 * recently it may be inaccurate.
 *
 * @param client    The client.
 * @return          Round trip time to the client in milliseconds.
 *
 */

int irmo_client_ping_time(IrmoClient *client);

/*!
 * Set the maximum send window size.
 *
 * This sets the maximum send window size for network transfer. This
 * is dependent on the bandwidth of the network connection. If the
 * maximum send window size is not set manually, the library will
 * attempt to determine an upper bound using congestion avoidance
 * algorithms, which may produce poor performance.
 *
 * @param client   The client to set the value on.
 * @param max      The maximum send window size (in bytes).
 */

void irmo_client_set_max_sendwindow(IrmoClient *client, unsigned int max);

/*!
 * Get the remote address of a client.
 *
 * This returns the remote address of a client in the form of
 * a presentable text string.  The form that this takes will differ,
 * depending on the type of connection (ie. the @ref IrmoNetModule
 * used to communicate with the client).  For IPv4 or IPv6, this
 * is the IP address of the client.
 *
 * @param client      The client to query.
 * @param buffer      Pointer to a buffer in which to store the
 *                    resulting string.
 * @param buffer_len  Size of the buffer, in bytes.  If the buffer is
 *                    too small to hold the whole address, it will
 *                    be truncated, but a valid C string will always
 *                    be created.
 */

void irmo_client_get_address(IrmoClient *client,
                             char *buffer, unsigned int buffer_len);

/*!
 * Get the port number of a client.
 *
 * This returns the port number used to communicate with a
 * remote client.  For IPv4 or IPv6, this is the UDP port number.
 *
 * @param client      The client.
 * @return            The client's remote port number.
 */

unsigned int irmo_client_get_port(IrmoClient *client);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_CLIENT_H */

