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
#include "base/iterator.h"

#include "interface/interface.h"

#include "class-callback-data.h"
#include "world.h"

IrmoWorld *irmo_world_new(IrmoInterface *iface)
{
	IrmoWorld *world;
	unsigned int i;

	irmo_return_val_if_fail(iface != NULL, NULL);
	
	world = irmo_new0(IrmoWorld, 1);

	world->iface = iface;
	world->objects = irmo_hash_table_new(irmo_pointer_hash, irmo_pointer_equal);
	world->refcount = 1;
	world->lastid = 0;
	world->servers = irmo_arraylist_new(1);
	world->remote = 0;
	
	irmo_interface_ref(iface);

        // Top-level class callbacks.

	irmo_class_callback_init(&world->callbacks_all, NULL, NULL);

	// Create a callback for each class
	
	world->callbacks = irmo_new0(ClassCallbackData, iface->nclasses);

	for (i=0; i<iface->nclasses; ++i) {
		ClassCallbackData *parent_data;
                IrmoClass *parent_class;

                parent_class = iface->classes[i]->parent_class;

		if (parent_class != NULL) {
			parent_data = &world->callbacks[parent_class->index];
		} else {
			parent_data = &world->callbacks_all;
		}

                irmo_class_callback_init(&world->callbacks[i],
                                         parent_data, 
                                         iface->classes[i]);
	}

	// method callbacks
	
	world->method_callbacks = irmo_new0(IrmoCallbackList, iface->nmethods);

	return world;
}

void irmo_world_ref(IrmoWorld *world)
{
	irmo_return_if_fail(world != NULL);
	
	++world->refcount;
}

static void irmo_world_destroy_all_objects(IrmoWorld *world)
{
        IrmoHashTableIterator *iter;
        IrmoObject *obj;

        iter = irmo_hash_table_iterate(world->objects);

        while (irmo_hash_table_iter_has_more(iter)) {
                obj = irmo_hash_table_iter_next(iter);
        
                irmo_object_internal_destroy(obj, 0, 0);
        }

        irmo_hash_table_iter_free(iter);
}

void irmo_world_unref(IrmoWorld *world)
{
        unsigned int i;

	irmo_return_if_fail(world != NULL);
	
	--world->refcount;

	if (world->refcount <= 0) {

		// delete list of servers (must by definition be empty
		// already)

		irmo_arraylist_free(world->servers);
		
		// destroy all objects and the objects hash table.
		
                irmo_world_destroy_all_objects(world);
		irmo_hash_table_free(world->objects);

		// delete callbacks
		
		for (i=0; i<world->iface->nclasses; ++i) {
			irmo_class_callback_free(&world->callbacks[i],
                                                 world->iface->classes[i]);
                }

		free(world->callbacks);

		irmo_class_callback_free(&world->callbacks_all, NULL);
		
		// method callbacks
		
		for (i=0; i<world->iface->nmethods; ++i)
			irmo_callback_list_free(&world->method_callbacks[i]);

		free(world->method_callbacks);

		// no longer using the interface 
		
		irmo_interface_unref(world->iface);
		
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

// Filter function used by irmo_world_iterate_objects when we
// only want to iterate over objects of a certain class.

static int irmo_object_iterator_filter(void *_obj, void *_klass)
{
        IrmoObject *obj = _obj;
        IrmoClass *klass = _klass;

        return irmo_object_is_a2(obj, klass);
}

IrmoIterator *irmo_world_iterate_objects(IrmoWorld *world, char *classname)
{
        IrmoIterator *iter;
	IrmoClass *klass;

	irmo_return_val_if_fail(world != NULL, NULL);
	
	if (classname) {
		klass = irmo_interface_get_class(world->iface, classname);

		if (!klass) {
			irmo_error_report("irmo_world_iterate_objects",
					  "unknown class '%s'", classname);
			return NULL;
		}
	} else {
		klass = NULL;
	}

        iter = irmo_iterate_hash_table(world->objects);

        if (klass != NULL) {
                irmo_iterator_set_filter(iter,
                                         irmo_object_iterator_filter,
                                         klass);
        }

        return iter;
}

IrmoInterface *irmo_world_get_interface(IrmoWorld *world)
{
	irmo_return_val_if_fail(world != NULL, NULL);

	return world->iface;
}

static ClassCallbackData *find_callback_class(IrmoWorld *world,
                                             char *classname,
                                             IrmoClass **save_class)
{
        IrmoClass *klass;

	if (classname == NULL) {
		return &world->callbacks_all;
        }

	klass = irmo_interface_get_class(world->iface, classname);

	if (klass == NULL) {
		return NULL;
        }

        if (save_class != NULL) {
                *save_class = klass;
        }

	return &world->callbacks[klass->index];
}


// Watch creation of new objects of a particular class

IrmoCallback *irmo_world_watch_new(IrmoWorld *world, char *classname,
                                   IrmoObjCallback func, void *user_data)
{
	ClassCallbackData *data;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	// find the class

	data = find_callback_class(world, classname, NULL);

	if (data == NULL) {
		irmo_error_report("irmo_world_watch_new",
                                  "unknown class '%s'", classname);
		return NULL;
	} else {
                return irmo_class_callback_watch_new(data, func, user_data);
	}
}

// Watch for changes to objects/variables

IrmoCallback *irmo_world_watch_class(IrmoWorld *world,
				     char *classname, char *variable,
				     IrmoVarCallback func, 
				     void *user_data)
{
	ClassCallbackData *data;
	IrmoCallback *callback = NULL;
        IrmoClass *klass;
	
	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	irmo_return_val_if_fail(!(classname == NULL && variable != NULL), NULL);
	
	// Find the class
	
	data = find_callback_class(world, classname, &klass);

	if (data == NULL) {
		irmo_error_report("irmo_world_watch_class",
				  "unknown class '%s'", classname);
                return NULL;
	}

        // Set the callback.

        callback = irmo_class_callback_watch(data, klass, variable,
                                             func, user_data);

        // If setting the callback failed, it was because the variable
        // specified was not found.

        if (callback == NULL) {
                irmo_error_report("irmo_world_watch_class",
                                  "unknown variable '%s' in class '%s'",
                                  variable, classname);
	}

	return callback;
}

// Watch for object destruction.

IrmoCallback *irmo_world_watch_destroy(IrmoWorld *world, 
				       char *classname,
				       IrmoObjCallback func, 
				       void *user_data)
{
	ClassCallbackData *data;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	data = find_callback_class(world, classname, NULL);

	if (data == NULL) {
		irmo_error_report("irmo_world_watch_destroy",
                                  "unknown class '%s'", classname);
                return NULL;
	} else {
		return irmo_class_callback_watch_destroy(data,
                                                         func,
                                                         user_data);
	}
}

