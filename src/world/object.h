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
// Irmo Objects
//

#ifndef IRMO_WORLD_OBJECT_H
#define IRMO_WORLD_OBJECT_H

#include <irmo/object.h>

#include "interface/interface.h"

#include "object-callback-data.h"

// internal stuff:

struct _IrmoObject {

	// world this object is attached to

	IrmoWorld *world;

	// callback data for this object
	
	ObjectCallbackData callbacks;

	// the class of this object, one of the classes specified
	// in the interface for the world this object exists
	// in.
	
	IrmoClass *objclass;

	// numerical id reference for this object in the world
	
	IrmoObjectID id;

	// array of variables for this object
	// the number of variables is specified in objclass
	
	IrmoValue *variables;

	// position in stream from remote server where variable
	// was last changed

	int *variable_time;

        // user-specified data set using _set_data and _get_data.

        void *user_data;
};

/*!
 * Internal function to create a new object.
 *
 * @param world           The world to create the object in.
 * @param objclass        Class of the new object.
 * @param id              Object ID to use for the new object.
 * @return                Pointer to the new object.
 */

IrmoObject *irmo_object_internal_new(IrmoWorld *world,
				     IrmoClass *objclass,
				     IrmoObjectID id);

/*!
 * Internal function to destroy an object.
 *
 * @param object          The object to destroy.
 * @param notify          If non-zero, notify routines are invoked for the
 *                        destruction of the object (notify connected clients,
 *                        invoke callbacks, etc).
 * @param remove          If non-zero, remove the object from the world's list
 *                        of objects.
 */

void irmo_object_internal_destroy(IrmoObject *object, int notify,
				  int remove);


/*!
 * Internal function to set the value of the specified variable.
 *
 * @param object          The object to set.
 * @param var             The variable to set.
 * @param value           The new value for the variable.
 */

void irmo_object_internal_set(IrmoObject *object, IrmoClassVar *var,
                              IrmoValue *value);

#endif /* #ifndef IRMO_WORLD_OBJECT_H */

