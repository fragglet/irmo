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
// Irmo World 
//

#ifndef IRMO_WORLD_WORLD_H
#define IRMO_WORLD_WORLD_H

#include <irmo/world.h>

#include "interface/interface.h"

#include "class-callback-data.h"
#include "object.h"

// internals:

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoWorld {

	// interface this world implements
	
	IrmoInterface *iface;

	// World-global callback objects, 1 per class
	// used for eg. "watch creation of objects of class my_class"
	// 		"watch for when my_class::my_string is changed"
	
	ClassCallbackData *callbacks;

	// global callbacks for all objects 
	// eg. "watch destruction of all objects"
	//     "watch for when any object is changed"

	ClassCallbackData callbacks_all;

	// objects in the world, hashed by their object id
	
	IrmoHashTable *objects;

	// the id of the last object created. objects are created
	// with sequential ids
	
	IrmoObjectID lastid;

	// servers attached to this world who are serving it.
	
	IrmoArrayList *servers;

	// number of references to this world
	
	int refcount;

	// if true, this is a local copy of a remote world and
	// cannot be changed
	
	int remote;

        // If this world is being served from a remote server, this
        // is the connection serving the world.

        IrmoConnection *source_connection;

	// method callbacks

	IrmoCallbackList *method_callbacks;
};

#endif /* #ifndef IRMO_WORLD_WORLD_H */

