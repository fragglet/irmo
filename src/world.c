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

//
// Irmo World 
//

#include <stdio.h>
#include <stdlib.h>

#include "callback.h"
#include "if_spec.h"
#include "world.h"

IrmoWorld *irmo_world_new(IrmoInterfaceSpec *spec)
{
	IrmoWorld *world;
	int i;

	g_return_val_if_fail(spec != NULL, NULL);
	
	world = g_new0(IrmoWorld, 1);

	world->spec = spec;
	world->objects = g_hash_table_new(g_direct_hash, g_direct_equal);
	world->refcount = 1;
	world->lastid = 0;
	world->servers = g_ptr_array_new();
	world->remote = FALSE;
	
	irmo_interface_spec_ref(spec);

	// create a callback for each class
	
	world->callbacks = g_new0(IrmoCallbackData *, spec->nclasses);

	for (i=0; i<spec->nclasses; ++i) {
		IrmoCallbackData *parent_data;

		if (spec->classes[i]->parent_class)
			parent_data = world->callbacks[spec->classes[i]->parent_class->index];
		else
			parent_data = NULL;
		
		world->callbacks[i] = irmo_callbackdata_new(spec->classes[i],
							    parent_data);
	}

	world->callbacks_all = irmo_callbackdata_new(NULL, NULL);

	// method callbacks
	
	world->method_callbacks = g_new0(GSList *, spec->nmethods);

	return world;
}

void irmo_world_ref(IrmoWorld *world)
{
	g_return_if_fail(world != NULL);
	
	++world->refcount;
}

static void irmo_world_unref_foreach(irmo_objid_t id, IrmoObject *object,
					gpointer user_data)
{
	// destroy object. do not notify objects. do not remove
	// from world as this may upset the foreach function
	// we are in.
	
	irmo_object_internal_destroy(object, FALSE, FALSE);
}

void irmo_world_unref(IrmoWorld *world)
{
	g_return_if_fail(world != NULL);
	
	--world->refcount;

	if (world->refcount <= 0) {
		int i;

		// delete list of servers (must by definition be empty
		// already)

		g_ptr_array_free(world->servers, 0);
		
		// delete all objects
		
		g_hash_table_foreach(world->objects,
				     (GHFunc) irmo_world_unref_foreach, 
				     NULL);
		g_hash_table_destroy(world->objects);

		// delete callbacks
		
		for (i=0; i<world->spec->nclasses; ++i)
			irmo_callbackdata_free(world->callbacks[i]);

		irmo_callbackdata_free(world->callbacks_all);
		
		free(world->callbacks);

		// no longer using the interface spec
		
		irmo_interface_spec_unref(world->spec);
		
		free(world);
	}
}

IrmoObject *irmo_world_get_object_for_id(IrmoWorld *world,
					    irmo_objid_t id)
{
	IrmoObject *object;

	g_return_if_fail(world != NULL);
	
	object = g_hash_table_lookup(world->objects, (gpointer) id);

	return object;
}

struct world_foreach_data {
	IrmoClass *spec;
	IrmoObjCallback func;
	gpointer user_data;
};

static void world_foreach_foreach(gint key,
				     IrmoObject *object,
				     struct world_foreach_data *data)
{
	// only call callback if this is of the particular class
	// or if no class was specified
	
	if (!data->spec || irmo_object_is_a2(object, data->spec)) {
		data->func(object, data->user_data);
	}
}
					    

void irmo_world_foreach_object(IrmoWorld *world, gchar *classname,
				  IrmoObjCallback func, gpointer user_data)
{
	IrmoClass *spec;
	struct world_foreach_data data = {
		func: func,
		user_data: user_data,
	};

	g_return_if_fail(world != NULL);
	g_return_if_fail(func != NULL);
	
	if (classname) {
		spec = irmo_interface_spec_get_class(world->spec, classname);

		if (!spec) {
			irmo_error_report("irmo_world_foreach_object",
					  "unknown class '%s'", classname);
			return;
		}
	} else {
		spec = NULL;
	}

	data.spec = spec;
	
	g_hash_table_foreach(world->objects,
			     (GHFunc) world_foreach_foreach,
			     &data);			     
}

IrmoInterfaceSpec *irmo_world_get_spec(IrmoWorld *world)
{
	g_return_val_if_fail(world != NULL, NULL);

	return world->spec;
}

// $Log$
// Revision 1.3  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.2  2003/09/02 20:33:55  fraggle
// Subclassing in interfaces
//
// Revision 1.1  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.6  2003/08/28 16:43:45  fraggle
// Use the reflection API internally to improve readability in places
//
// Revision 1.5  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.4  2003/08/16 16:45:11  fraggle
// Allow watches on all objects regardless of class
//
// Revision 1.3  2003/08/15 17:53:55  fraggle
// irmo_object_get_universe, irmo_universe_get_spec functions
//
// Revision 1.2  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.17  2003/06/09 21:06:53  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.16  2003/04/25 01:26:19  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.15  2003/03/15 02:21:16  sdh300
// Initial method code
//
// Revision 1.14  2003/03/07 12:17:18  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.13  2003/03/06 20:53:16  sdh300
// Checking of remote flag for universe objects
//
// Revision 1.12  2003/03/06 19:33:51  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.11  2003/03/06 19:22:17  sdh300
// Split off some of the constructor/destructor/change code into
// seperate functions that can be reused elsewhere
//
// Revision 1.10  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.9  2003/02/18 18:39:54  sdh300
// store list of attached servers in universe
//
// Revision 1.8  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.7  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.6  2002/10/21 15:09:01  sdh300
// object destruction
//
// Revision 1.5  2002/10/21 14:58:07  sdh300
// split off object code to a seperate module
//
// Revision 1.4  2002/10/21 14:48:54  sdh300
// oops, fix build
//
// Revision 1.3  2002/10/21 14:43:26  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
