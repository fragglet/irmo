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
// Irmo World 
//

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "interface/interface.h"

#include "callback.h"
#include "world.h"

IrmoWorld *irmo_world_new(IrmoInterface *spec)
{
	IrmoWorld *world;
	int i;

	irmo_return_val_if_fail(spec != NULL, NULL);
	
	world = irmo_new0(IrmoWorld, 1);

	world->spec = spec;
	world->objects = irmo_hash_table_new(irmo_pointer_hash, irmo_pointer_equal);
	world->refcount = 1;
	world->lastid = 0;
	world->servers = irmo_arraylist_new(1);
	world->remote = 0;
	
	irmo_interface_ref(spec);

	// create a callback for each class
	
	world->callbacks = irmo_new0(IrmoCallbackData *, spec->nclasses);

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
	
	world->method_callbacks = irmo_new0(IrmoSListEntry *, spec->nmethods);

	return world;
}

void irmo_world_ref(IrmoWorld *world)
{
	irmo_return_if_fail(world != NULL);
	
	++world->refcount;
}

static void irmo_world_unref_foreach(IrmoObjectID id, IrmoObject *object,
				     void *user_data)
{
	// destroy object. do not notify objects. do not remove
	// from world as this may upset the foreach function
	// we are in.
	
	irmo_object_internal_destroy(object, 0, 0);
}

void irmo_world_unref(IrmoWorld *world)
{
	irmo_return_if_fail(world != NULL);
	
	--world->refcount;

	if (world->refcount <= 0) {
		int i;

		// delete list of servers (must by definition be empty
		// already)

		irmo_arraylist_free(world->servers);
		
		// delete all objects
		
		irmo_hash_table_foreach(world->objects,
				     (IrmoHashTableIterator) irmo_world_unref_foreach, 
				     NULL);
		irmo_hash_table_free(world->objects);

		// delete callbacks
		
		for (i=0; i<world->spec->nclasses; ++i)
			irmo_callbackdata_free(world->callbacks[i]);

		irmo_callbackdata_free(world->callbacks_all);
		
		free(world->callbacks);

		// method callbacks
		
		for (i=0; i<world->spec->nmethods; ++i)
			irmo_callbacklist_free(world->method_callbacks[i]);

		free(world->method_callbacks);

		// no longer using the interface spec
		
		irmo_interface_unref(world->spec);
		
		free(world);
	}
}

IrmoObject *irmo_world_get_object_for_id(IrmoWorld *world,
					 IrmoObjectID id)
{
	IrmoObject *object;

	irmo_return_val_if_fail(world != NULL, NULL);
	
	object = irmo_hash_table_lookup(world->objects, (void *) id);

	return object;
}

struct world_foreach_data {
	IrmoClass *spec;
	IrmoObjCallback func;
	void *user_data;
};

static void world_foreach_foreach(int key,
				  IrmoObject *object,
				  struct world_foreach_data *data)
{
	// only call callback if this is of the particular class
	// or if no class was specified
	
	if (!data->spec || irmo_object_is_a2(object, data->spec)) {
		data->func(object, data->user_data);
	}
}
					    

void irmo_world_foreach_object(IrmoWorld *world, char *classname,
			       IrmoObjCallback func, void *user_data)
{
	IrmoClass *spec;
	struct world_foreach_data data = {
		NULL,
		func,
		user_data,
	};

	irmo_return_if_fail(world != NULL);
	irmo_return_if_fail(func != NULL);
	
	if (classname) {
		spec = irmo_interface_get_class(world->spec, classname);

		if (!spec) {
			irmo_error_report("irmo_world_foreach_object",
					  "unknown class '%s'", classname);
			return;
		}
	} else {
		spec = NULL;
	}

	data.spec = spec;
	
	irmo_hash_table_foreach(world->objects,
			     (IrmoHashTableIterator) world_foreach_foreach,
			     &data);			     
}

IrmoInterface *irmo_world_get_spec(IrmoWorld *world)
{
	irmo_return_val_if_fail(world != NULL, NULL);

	return world->spec;
}

