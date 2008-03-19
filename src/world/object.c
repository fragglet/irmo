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
// Object code
//

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "net/client.h"
#include "net/sendatom.h"

#include "object.h"
#include "world.h"

// do something for all connected clients

typedef void (*ClientCallback) (IrmoClient *client, void *user_data);

static void server_foreach_client(IrmoServer *server,
                                  ClientCallback func,
                                  void *user_data)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {
                client = irmo_hash_table_iter_next(iter);

                // Ignore clients that are not fully connected

                if (client->state == CLIENT_CONNECTED) {
                        func(client, user_data);
                }
        }

        irmo_hash_table_iter_free(iter);
}

static void foreach_client(IrmoWorld *world,
			   ClientCallback func,
                           void *user_data)
{
	int i;

	for (i=0; i<world->servers->length; ++i) {
		IrmoServer *server
			= (IrmoServer *) world->servers->data[i];

                server_foreach_client(server, func, user_data);
	}
}		   

static int get_free_id(IrmoWorld *world)
{
	IrmoObjectID start = world->lastid;

	// keep incrementing until we find one free
	// we increment once to start off with, since the current lastid
	// was assigned to the previous object
	
	do {
		world->lastid = (world->lastid + 1) % MAX_OBJECTS;

		// no free spaces
		
		if (world->lastid == start)
			return -1;
	} while (irmo_hash_table_lookup(world->objects,
				        IRMO_POINTER_KEY(world->lastid)));

	return world->lastid;
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

	// notify attached clients

	foreach_client(world,
		       (ClientCallback) irmo_client_sendq_add_new, object);

	// if a remote world, create variable_time array

	if (world->remote) {
		object->variable_time = irmo_new0(int, objclass->nvariables);
	}

	return object;
}
				

IrmoObject *irmo_object_new(IrmoWorld *world, char *type_name)
{
	IrmoClass *klass;
	int id;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(type_name != NULL, NULL);
	irmo_return_val_if_fail(world->remote == 0, NULL);
	
	klass = irmo_interface_get_class(world->iface, type_name);

	if (klass == NULL) {
		irmo_error_report("irmo_object_new", 
				  "unknown type '%s'", type_name);
		return NULL;
	}

	id = get_free_id(world);

	if (id < 0) {
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
        ClassCallbackData *class_data;
	unsigned int i;

	if (notify) {
		// raise destroy callbacks
		
                irmo_object_callback_raise_destroy(&object->callbacks, object);

                class_data = &object->world->callbacks[object->objclass->index];
                irmo_class_callback_raise_destroy(class_data, object);
		
		// notify connected clients
		
		foreach_client(object->world,
			       (ClientCallback) irmo_client_sendq_add_destroy,
			       object);
	}	

	// remove from world

	if (remove) {
		irmo_hash_table_remove(object->world->objects,
				       IRMO_POINTER_KEY(object->id));
	}
	
	// destroy member variables

	for (i=0; i<object->objclass->nvariables; ++i) {
		if (object->objclass->variables[i]->type == IRMO_TYPE_STRING
		 && object->variables[i].s) {
			free(object->variables[i].s);
                }
	}

	free(object->variables);
	irmo_object_callback_free(&object->callbacks, object->objclass);

	// free variable time array

	if (object->variable_time != NULL) {
		free(object->variable_time);
	}

	// done
	
	free(object);
}

void irmo_object_destroy(IrmoObject *object)
{
	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(object->world->remote == 0);
	
	// destroy object
	// notify callbacks and remove from world
	
	irmo_object_internal_destroy(object, 1, 1);
}

IrmoObjectID irmo_object_get_id(IrmoObject *object)
{
	irmo_return_val_if_fail(object != NULL, -1);
	
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

// notify connected clients of changes

struct set_notify_data {
	IrmoObject *object;
	int variable;
};

static void object_set_notify_foreach(IrmoClient *client,
				      struct set_notify_data *data)
{
	irmo_client_sendq_add_change(client, data->object, data->variable);
}

// call callback functions and notify clients when a variable is changed

void irmo_object_set_raise(IrmoObject *object, int variable)
{
	IrmoClass *objclass = object->objclass;
	IrmoClassVar *var = objclass->variables[variable];
        ClassCallbackData *class_data;
	struct set_notify_data data = {
		object,
		variable,
	};

	// call callback functions for change

        irmo_object_callback_raise(&object->callbacks, object, var->index);

        class_data = &object->world->callbacks[objclass->index];
        irmo_class_callback_raise(class_data, object, var->index);

	// notify clients

	foreach_client(object->world,
		       (ClientCallback) object_set_notify_foreach,
		       &data);
}

void irmo_object_set(IrmoObject *object, IrmoClassVar *variable,
                     IrmoValue *value)
{
        IrmoValue *obj_value;

	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	irmo_return_if_fail(value != NULL);
	irmo_return_if_fail(!object->world->remote);

        // Object must be of the right class

        irmo_return_if_fail(irmo_object_is_a2(object, variable->klass));

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
		irmo_error_report("irmo_object_set",
				  "variable has unknown type");
                return;
        }

	irmo_object_set_raise(object, variable->index);
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
		irmo_error_report("irmo_object_set_int",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		return;
	}

        // Check it is an integer type

        if (var->type != IRMO_TYPE_INT8 && var->type != IRMO_TYPE_INT16
         && var->type != IRMO_TYPE_INT32) {
		irmo_error_report("irmo_object_set_int",
				  "variable '%s' in class '%s' is not an int type",
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
		irmo_error_report("irmo_object_set_string",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		
		return;
	}

	if (var->type != IRMO_TYPE_STRING) {
		irmo_error_report("irmo_object_set_string",
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

	irmo_return_val_if_fail(object != NULL, -1);
	irmo_return_val_if_fail(variable != NULL, -1);
	
	var = irmo_class_get_variable(object->objclass, variable);

	if (var == NULL) {
		irmo_error_report("irmo_object_get_int",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		
		return -1;
	}

	switch (var->type) {
	case IRMO_TYPE_INT8:
	case IRMO_TYPE_INT16:
	case IRMO_TYPE_INT32:
		return object->variables[var->index].i;
	default:
		irmo_error_report("irmo_object_get_int",
				  "variable '%s' in class '%s' is not an int type",
				  variable, object->objclass->name);
		return -1;
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
		irmo_error_report("irmo_object_get_string",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		
		return NULL;
	}

	if (var->type != IRMO_TYPE_STRING) {
		irmo_error_report("irmo_object_get_string",
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

unsigned int irmo_object_is_a2(IrmoObject *obj, IrmoClass *klass)
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

unsigned int irmo_object_is_a(IrmoObject *obj, char *classname)
{
	IrmoClass *klass;

	irmo_return_val_if_fail(obj != NULL, 0);
	irmo_return_val_if_fail(classname != NULL, 0);

	klass = irmo_interface_get_class(obj->world->iface, classname);

	if (klass == NULL) {
		irmo_error_report("irmo_object_is_a",
				  "unknown class name '%s'", classname);
		return 0;
	}

	return irmo_object_is_a2(obj, klass);
}

void irmo_object_set_data(IrmoObject *object, void *value)
{
        object->user_data = value;
}

void *irmo_object_get_data(IrmoObject *object)
{
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
		irmo_error_report("irmo_object_watch",
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

