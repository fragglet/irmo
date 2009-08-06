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

//
// C structure binding for Irmo objects.
//

#ifndef IRMO_WORLD_BINDING_H
#define IRMO_WORLD_BINDING_H

#include "object.h"

/*!
 * Use the specified variable of the specified object to update the object's
 * C structure binding.
 *
 * @param obj           The object that has been changed.
 * @param class_var     The variable of the object that has changed.
 */

void irmo_object_update_binding(IrmoObject *obj, IrmoClassVar *class_var);

#endif /* #ifndef IRMO_WORLD_BINDING_H */

