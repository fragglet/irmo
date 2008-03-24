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

#ifndef IRMO_NET_SERVER_WORLD_H
#define IRMO_NET_SERVER_WORLD_H

#include <irmo/types.h>

/*!
 * Function invoked when a new object is created in a world being served
 * by a server.
 *
 * @param server             The server serving the world.
 * @param obj                The new object being created.
 */

extern void irmo_server_object_new(IrmoServer *server, IrmoObject *obj);

/*!
 * Function invoked when an object is destroyed in a world being served
 * by a server.
 *
 * @param server             The server serving the world.
 * @param obj                The new object being destroyed.
 */

extern void irmo_server_object_destroyed(IrmoServer *server, IrmoObject *obj);

/*!
 * Function invoked when a variable is changed in an object that is
 * part of a world being served by a server.
 *
 * @param server             The server serving the world.
 * @param obj                The new object being changed.
 * @param classvar           The variable being changed.
 */

extern void irmo_server_object_changed(IrmoServer *server, IrmoObject *obj,
                                       IrmoClassVar *var);

/*!
 * Function invoked when a method is invoked on a world being served
 * from a remote server.
 *
 * @param connection         The connection serving the world.
 * @param method_data        Data for the method call (argument values, etc)
 */

extern void irmo_connection_method_call(IrmoConnection *conn, 
                                        IrmoMethodData *data);

#endif /* #ifndef IRMO_NET_SERVER_WORLD_H */

