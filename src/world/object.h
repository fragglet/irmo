//
// Copyright (C) 2002-3 Simon Howard
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

#ifndef IRMO_INTERNAL_OBJECT_H
#define IRMO_INTERNAL_OBJECT_H

#include <irmo/object.h>

#include "interface/interface.h"

#include "callback-data.h"

// internal stuff:

struct _IrmoObject {

	// world this object is attached to

	IrmoWorld *world;

	// callback data for this object
	
	IrmoCallbackData *callbacks;

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

// internal function create a new object

IrmoObject *irmo_object_internal_new(IrmoWorld *world,
				     IrmoClass *objclass,
				     IrmoObjectID id);

// internal function to destroy an object. control over whether to
// call notify routines (call callbacks, forward info to clients)
// and whether to remove from the world.

void irmo_object_internal_destroy(IrmoObject *object, int notify,
				  int remove);


// internal function to raise notify functions for when an object
// is modified. callback functions are called and the change forwarded
// to connected clients. needs object and index of variable number
// modified.

void irmo_object_set_raise(IrmoObject *object, int variable);


#endif /* #ifndef IRMO_OBJECT_H */

