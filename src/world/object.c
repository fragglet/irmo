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
// Object code
//

#include "arch/sysheaders.h"
#include "base/alloc.h"
#include "base/assert.h"
#include "base/error.h"

#include "net/server-world.h"

#include "binding.h"
#include "object.h"
#include "world.h"

// Get the next free object ID for the specified world.
//
// Returns true if an ID was found.

static int get_free_id(IrmoWorld *world, IrmoObjectID *id)
{
	IrmoObjectID start = world->lastid;

	// keep incrementing until we find one free
	// we increment once to start off with, since the current lastid
	// was assigned to the previous object

	do {
		world->lastid = (world->lastid + 1) % MAX_OBJECTS;

		// no free spaces

		if (world->lastid == start) {
			return 0;
                }

	} while (irmo_hash_table_lookup(world->objects,
				        IRMO_POINTER_KEY(world->lastid)));

        *id = world->lastid;

        return 1;
}

IrmoObject *irmo_object_internal_new(IrmoWorld *world,
				     IrmoClass *objclass,
				     IrmoObjectID id)
{
	IrmoObject *object;
        unsigned int i;

	// make object
	
	object = irmo_new0(IrmoObject, 1);

	object->id = id;
	object->objclass = objclass;
	object->world = world;

        irmo_object_callback_init(&object->callbacks, objclass);
	
	// member variables:

	object->variables = irmo_new0(IrmoValue, objclass->nvariables);

	// int variables will be initialised to 0 by irmo_new0
	// string values must be initialised to the empty string ("")
	
	for (i=0; i<objclass->nvariables; ++i) {
		if (objclass->variables[i]->type == IRMO_TYPE_STRING) {
			object->variables[i].s = strdup("");
                }
        }
	
	// add to world

	irmo_hash_table_insert(world->objects, IRMO_POINTER_KEY(id), object);

	// raise callback functions for new object creation

	irmo_class_callback_raise_new(&world->callbacks[objclass->index],
                                      object);

	// Notify servers attached to this world of the new object.

        for (i=0; i<world->servers->length; ++i) {
                irmo_server_object_new(world->servers->data[i], object);
        }

	// if a remote world, create variable_time array

        if (world->remote) {
                object->variable_time = irmo_new0(unsigned int,
                                                  objclass->nvariables);
        }

	return object;
}
				

IrmoObject *irmo_object_new(IrmoWorld *world, char *type_name)
{
	IrmoClass *klass;
	IrmoObjectID id;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(type_name != NULL, NULL);
	irmo_return_val_if_fail(world->remote == 0, NULL);
	
	klass = irmo_interface_get_class(world->iface, type_name);

	if (klass == NULL) {
                irmo_warning_message("irmo_object_new",
				     "unknown type '%s'", type_name);
		return NULL;
	}

        // Try to find a new object ID.

        if (!get_free_id(world, &id)) {
		irmo_error_report("irmo_object_new",
                        "maximum of %i objects per world (no more objects!)",
                        MAX_OBJECTS);

		return NULL;
	}

	return irmo_object_internal_new(world, klass, id);
}

// internal object destroy function

void irmo_object_internal_destroy(IrmoObject *object,
				  int notify,
				  int remove)
{
        IrmoWorld *world;
        ClassCallbackData *class_data;
	unsigned int i;

	if (notify) {
		// raise destroy callbacks
		
                irmo_object_callback_raise_destroy(&object->callbacks, object);

                class_data = &object->world->callbacks[object->objclass->index];
                irmo_class_callback_raise_destroy(class_data, object);
		
		// notify connected clients
		
                world = object->world;

                for (i=0; i<world->servers->length; ++i) {
                        irmo_server_object_destroyed(world->servers->data[i],
                                                     object);
                }
	}	

	// remove from world

	if (remove) {
		irmo_hash_table_remove(object->world->objects,
				       IRMO_POINTER_KEY(object->id));
	}
	
	// destroy member variables

	for (i=0; i<object->objclass->nvariables; ++i) {
		if (object->objclass->variables[i]->type == IRMO_TYPE_STRING) {
			free(object->variables[i].s);
                }
	}

	free(object->variables);
	irmo_object_callback_free(&object->callbacks, object->objclass);

	// free variable time array

        free(object->variable_time);

	// done
	
	free(object);
}

void irmo_object_destroy(IrmoObject *object)
{
	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(!object->world->remote);
	
	// destroy object
	// notify callbacks and remove from world
	
	irmo_object_internal_destroy(object, 1, 1);
}

IrmoObjectID irmo_object_get_id(IrmoObject *object)
{
	irmo_return_val_if_fail(object != NULL, 0);
	
	return object->id;
}

char *irmo_object_get_class(IrmoObject *object)
{
	irmo_return_val_if_fail(object != NULL, NULL);
	
	return object->objclass->name;
}

IrmoClass *irmo_object_get_class_obj(IrmoObject *object)
{
	irmo_return_val_if_fail(object != NULL, NULL);

	return object->objclass;
}

// call callback functions and notify clients when a variable is changed

static void irmo_object_set_raise(IrmoObject *object, IrmoClassVar *var)
{
	IrmoClass *objclass = object->objclass;
        IrmoWorld *world;
        ClassCallbackData *class_data;
        unsigned int i;

	// call callback functions for change

        irmo_object_callback_raise(&object->callbacks, object,
                                   object->objclass, var->index);

        class_data = &object->world->callbacks[objclass->index];
        irmo_class_callback_raise(class_data, object, var->index);

	// notify clients

        world = object->world;

        for (i=0; i<world->servers->length; ++i) {
                irmo_server_object_changed(world->servers->data[i],
                                           object, var);
        }
}

void irmo_object_internal_set(IrmoObject *object,
                              IrmoClassVar *variable,
                              IrmoValue *value,
                              int update_binding)
{
        IrmoValue *obj_value;

	obj_value = &object->variables[variable->index];

        // Set the variable

	switch (variable->type) {
	case IRMO_TYPE_INT8:
		irmo_return_if_fail(value->i <= 0xff);
		obj_value->i = value->i;
		break;
	case IRMO_TYPE_INT16:
		irmo_return_if_fail(value->i <= 0xffff);
		obj_value->i = value->i;
		break;
	case IRMO_TYPE_INT32:
		obj_value->i = value->i;
		break;
        case IRMO_TYPE_STRING:
                free(obj_value->s);
                obj_value->s = strdup(value->s);
                break;
        default:
                irmo_bug();
        }

        // If the object has a binding, update the structure member
        // for this variable.

        if (update_binding && object->binding != NULL) {
                irmo_object_update_binding(object, variable);
        }

        // Invoked callback functions:

	irmo_object_set_raise(object, variable);
}

void irmo_object_set(IrmoObject *object, IrmoClassVar *variable,
                     IrmoValue *value)
{
	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	irmo_return_if_fail(value != NULL);
	irmo_return_if_fail(!object->world->remote);

        // Object must be of the right class

        irmo_return_if_fail(irmo_object_is_a2(object, variable->klass));

        // Set the value

        irmo_object_internal_set(object, variable, value, 1);
}


void irmo_object_set_int(IrmoObject *object, char *variable, 
			 unsigned int value)
{
	IrmoClassVar *var;
	IrmoValue irmo_value;

	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	
	var = irmo_class_get_variable(object->objclass, variable);

        if (var == NULL) {
                irmo_warning_message("irmo_object_set_int",
                                     "unknown variable '%s' in class '%s'",
                                     variable,
                                     object->objclass->name);
                return;
        }

        // Check it is an integer type

        if (var->type != IRMO_TYPE_INT8 && var->type != IRMO_TYPE_INT16
         && var->type != IRMO_TYPE_INT32) {
                irmo_warning_message("irmo_object_set_int",
                        "variable '%s' in class '%s' is not an integer type",
                        variable, object->objclass->name);
                return;
        }

        // Set the value

	irmo_value.i = value;
        irmo_object_set(object, var, &irmo_value);
}

void irmo_object_set_string(IrmoObject *object, char *variable, char *value)
{
	IrmoClassVar *var;
        IrmoValue irmo_value;

	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	irmo_return_if_fail(value != NULL);
	
	var = irmo_class_get_variable(object->objclass, variable);

	if (var == NULL) {
                irmo_warning_message("irmo_object_set_string",
                                     "unknown variable '%s' in class '%s'",
                                     variable,
                                     object->objclass->name);
		
                return;
	}

	if (var->type != IRMO_TYPE_STRING) {
                irmo_warning_message("irmo_object_set_string",
                        "variable '%s' in class '%s' is not string type",
                        variable, object->objclass->name);
                return;
	}

        // Set the new value

        irmo_value.s = value;
        irmo_object_set(object, var, &irmo_value);
}

void irmo_object_get(IrmoObject *object, IrmoClassVar *variable, 
                     IrmoValue *value)
{
	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	irmo_return_if_fail(value != NULL);
	
        // Check this variable is in the class of this object

        irmo_return_if_fail(irmo_object_is_a2(object, variable->klass));

        // Return the value

        *value = object->variables[variable->index];
}

// get int value

unsigned int irmo_object_get_int(IrmoObject *object, char *variable)
{
	IrmoClassVar *var;

	irmo_return_val_if_fail(object != NULL, 0);
	irmo_return_val_if_fail(variable != NULL, 0);
	
	var = irmo_class_get_variable(object->objclass, variable);

	if (var == NULL) {
                irmo_warning_message("irmo_object_get_int",
                                     "unknown variable '%s' in class '%s'",
                                     variable,
                                     object->objclass->name);
		
                return 0;
	}

	switch (var->type) {
	case IRMO_TYPE_INT8:
	case IRMO_TYPE_INT16:
	case IRMO_TYPE_INT32:
		return object->variables[var->index].i;
	default:
		irmo_warning_message("irmo_object_get_int",
                        "variable '%s' in class '%s' is not an integer type",
                        variable, object->objclass->name);

		return 0;
	}
}

// get int value

char *irmo_object_get_string(IrmoObject *object, char *variable)
{
	IrmoClassVar *var;

	irmo_return_val_if_fail(object != NULL, NULL);
	irmo_return_val_if_fail(variable != NULL, NULL);

	var = irmo_class_get_variable(object->objclass, variable);

	if (var == NULL) {
                irmo_warning_message("irmo_object_get_string",
                                     "unknown variable '%s' in class '%s'",
                                     variable,
                                     object->objclass->name);
		
                return NULL;
	}

	if (var->type != IRMO_TYPE_STRING) {
                irmo_warning_message("irmo_object_get_string",
                        "variable '%s' in class '%s' is not a string type",
                        variable, object->objclass->name);
                return NULL;
	}

	return object->variables[var->index].s;
}

IrmoWorld *irmo_object_get_world(IrmoObject *obj)
{
	irmo_return_val_if_fail(obj != NULL, NULL);

	return obj->world;
}

int irmo_object_is_a2(IrmoObject *obj, IrmoClass *klass)
{
	IrmoClass *c;
	
	irmo_return_val_if_fail(obj != NULL, 0);
	irmo_return_val_if_fail(klass != NULL, 0);

	// search through all parent classes

	for (c=obj->objclass; c; c=c->parent_class) {
		if (c == klass) {
			return 1;
                }
	}

	return 0;
}

int irmo_object_is_a(IrmoObject *obj, char *classname)
{
	IrmoClass *klass;

	irmo_return_val_if_fail(obj != NULL, 0);
	irmo_return_val_if_fail(classname != NULL, 0);

	klass = irmo_interface_get_class(obj->world->iface, classname);

	if (klass == NULL) {
                irmo_warning_message("irmo_object_is_a",
                                     "unknown class name '%s'", classname);
                return 0;
	}

	return irmo_object_is_a2(obj, klass);
}

void irmo_object_set_data(IrmoObject *object, void *value)
{
        irmo_return_if_fail(object != NULL);

        object->user_data = value;
}

void *irmo_object_get_data(IrmoObject *object)
{
        irmo_return_val_if_fail(object != NULL, NULL);

        return object->user_data;
}

IrmoCallback *irmo_object_watch(IrmoObject *object, char *variable,
				IrmoVarCallback func, void *user_data)
{
        IrmoCallback *callback;

	irmo_return_val_if_fail(object != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);

        callback = irmo_object_callback_watch(&object->callbacks,
                                              object->objclass,
                                              variable,
                                              func,
                                              user_data);

	if (callback == NULL) {
                irmo_warning_message("irmo_object_watch",
                        "unknown variable '%s' in class '%s'",
                        variable, object->objclass->name);
	}

	return callback;
}

IrmoCallback *irmo_object_watch_destroy(IrmoObject *object,
					IrmoObjCallback func, 
					void *user_data)
{
	irmo_return_val_if_fail(object != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	return irmo_object_callback_watch_destroy(&object->callbacks,
                                                  func,
                                                  user_data);
}

