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

#ifndef IRMO_NET_CLIENT_SENDQ_H
#define IRMO_NET_CLIENT_SENDQ_H

/*!
 * Add an atom to the specified client's send queue to signal that
 * a new object has been created.
 *
 * @param client              The client.
 * @param object              The newly-created object.
 */

void irmo_client_sendq_add_new(IrmoClient *client, IrmoObject *object);

/*!
 * Add an atom to the specified client's send queue to signal that a
 * variable that is part of an object has been changed.
 *
 * @param client              The client.
 * @param object              The object.
 * @param var                 The variable that has changed.
 */

void irmo_client_sendq_add_change(IrmoClient *client, IrmoObject *object, 
                                  IrmoClassVar *var);

/*!
 * Add an atom to the specified client's send queue to signal that the
 * specified object has been destroyed.
 *
 * @param client              The client.
 * @param object              The object being destroyed.
 */

void irmo_client_sendq_add_destroy(IrmoClient *client, IrmoObject *object);

/*!
 * Add an atom to the specified client's send queue to invoke a method
 * on the remote world being served.
 *
 * @param client              The client.
 * @param data                Structure describing the method call.
 */

void irmo_client_sendq_add_method(IrmoClient *client, IrmoMethodData *data);

/*!
 * Add an atom to the specified client's send queue to set the send
 * window size.
 *
 * @param client              The client.
 * @param max                 The new maximum send queue size.
 */

void irmo_client_sendq_add_sendwindow(IrmoClient *client, int max);

/*!
 * Remove an atom from the send queue.
 *
 * @param client              The client.
 * @return                    A send atom, or NULL if there are no atoms
 *                            waiting in the queue.
 */

IrmoSendAtom *irmo_client_sendq_pop(IrmoClient *client);

/*!
 * Queue up the entire state of the world being served for transmission
 * to the remote client.
 *
 * @param client              The client.
 */

void irmo_client_sendq_add_state(IrmoClient *client);

#endif /* #ifndef IRMO_NET_CLIENT_SENDQ_H */

