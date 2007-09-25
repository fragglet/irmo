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

#include "sysheaders.h"

#include "callback.h"
#include "client.h"
#include "error.h"
#include "object.h"
#include "sendatom.h"
#include "world.h"

// do something for all connected clients

typedef void (*ClientCallback) (IrmoClient *client, void *user_data);

struct foreach_client_data {
	IrmoWorld *world;
	ClientCallback func;
	void *user_data;
};

static void foreach_client_foreach(void *key, IrmoClient *client,
				   struct foreach_client_data *data)
{
	// only do this for clients which are full connected

	if (client->state != CLIENT_CONNECTED)
		return;

	data->func(client, data->user_data);
}

static void foreach_client(IrmoWorld *world,
			   ClientCallback func, void *user_data)
{
	struct foreach_client_data data = {
		world,
		func,
		user_data,
	};
	int i;

	for (i=0; i<world->servers->length; ++i) {
		IrmoServer *server
			= (IrmoServer *) world->servers->data[i];

		irmo_hash_table_foreach(server->clients,
				     (IrmoHashTableIterator) foreach_client_foreach,
				     &data);
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
				     (void *) world->lastid));

	return world->lastid;
}

IrmoObject *irmo_object_internal_new(IrmoWorld *world,
				     IrmoClass *objclass,
				     IrmoObjectID id)
{
	IrmoObject *object;
	int i;

	// make object
	
	object = irmo_new0(IrmoObject, 1);

	object->id = id;
	object->objclass = objclass;
	object->world = world;
	object->callbacks = irmo_callbackdata_new(objclass, NULL);
	
	// member variables:

	object->variables = irmo_new0(IrmoValue, objclass->nvariables);

	// int variables will be initialised to 0 by irmo_new0
	// string values must be initialised to the empty string ("")
	
	for (i=0; i<objclass->nvariables; ++i)
		if (objclass->variables[i]->type == IRMO_TYPE_STRING)
			object->variables[i].s = strdup("");

	
	// add to world

	irmo_hash_table_insert(world->objects, (void *) id, object);

	// raise callback functions for new object creation

	irmo_callbackdata_raise_new(world->callbacks[objclass->index],
				    object);
	irmo_callbackdata_raise_new(world->callbacks_all, object);

	// notify attached clients

	foreach_client(world,
		       (ClientCallback) irmo_client_sendq_add_new, object);

	// if a remote world, create variable_time array

	if (world->remote)
		object->variable_time = irmo_new0(int, objclass->nvariables);
	
	return object;
}
				

IrmoObject *irmo_object_new(IrmoWorld *world, char *type_name)
{
	IrmoClass *spec;
	int id;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(type_name != NULL, NULL);
	irmo_return_val_if_fail(world->remote == 0, NULL);
	
	spec = irmo_interface_spec_get_class(world->spec, type_name);

	if (!spec) {
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

	return irmo_object_internal_new(world, spec, id);
}

// internal object destroy function

void irmo_object_internal_destroy(IrmoObject *object,
				  int notify,
				  int remove)
{
	int i;

	if (notify) {
		// raise destroy callbacks
		
		irmo_callbackdata_raise_destroy(object->callbacks, object);
		irmo_callbackdata_raise_destroy(object->world->callbacks
						   [object->objclass->index],
						object);
		irmo_callbackdata_raise_destroy(object->world->callbacks_all,
						object);
		
		// notify connected clients
		
		foreach_client(object->world,
			       (ClientCallback) irmo_client_sendq_add_destroy,
			       object);
	}	

	// remove from world

	if (remove) {
		irmo_hash_table_remove(object->world->objects,
				    (void *) object->id);
	}
	
	// destroy member variables

	for (i=0; i<object->objclass->nvariables; ++i) {
		if (object->objclass->variables[i]->type == IRMO_TYPE_STRING
		    && object->variables[i].s)
			free(object->variables[i].s);
	}

	free(object->variables);
	irmo_callbackdata_free(object->callbacks);

	// free variable time array

	if (object->variable_time)
		free(object->variable_time);
	
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
	IrmoClassVar *spec = objclass->variables[variable];
	struct set_notify_data data = {
		object,
		variable,
	};

	// call callback functions for change

	irmo_callbackdata_raise(object->callbacks, object, spec->index);
	irmo_callbackdata_raise(object->world->callbacks[objclass->index],
				object, variable);
	irmo_callbackdata_raise(object->world->callbacks_all,
				object, variable);
	
	// notify clients

	foreach_client(object->world,
		       (ClientCallback) object_set_notify_foreach,
		       &data);
}

void irmo_object_set_int(IrmoObject *object, char *variable, 
			 unsigned int value)
{
	IrmoClassVar *spec;
	IrmoValue *obj_var;

	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	irmo_return_if_fail(object->world->remote == 0);
	
	spec = irmo_class_get_variable(object->objclass, variable);

	if (!spec) {
		irmo_error_report("irmo_object_set_int",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		return;
	}

	obj_var = &object->variables[spec->index];

	switch (spec->type) {
	case IRMO_TYPE_INT8:
		irmo_return_if_fail(value >= 0 && value <= 0xff);
		obj_var->i = value;
		break;
	case IRMO_TYPE_INT16:
		irmo_return_if_fail(value >= 0 && value <= 0xffff);
		obj_var->i = value;
		break;
	case IRMO_TYPE_INT32:
		obj_var->i = value;
		break;
	default:
		irmo_error_report("irmo_object_set_int",
				  "variable '%s' in class '%s' is not an int type",
				  variable, object->objclass->name);
		return;
	}

	irmo_object_set_raise(object, spec->index);
}

void irmo_object_set_string(IrmoObject *object, char *variable, char *value)
{
	IrmoClassVar *spec;

	irmo_return_if_fail(object != NULL);
	irmo_return_if_fail(variable != NULL);
	irmo_return_if_fail(value != NULL);
	irmo_return_if_fail(object->world->remote == 0);
	
	spec = irmo_class_get_variable(object->objclass, variable);

	if (!spec) {
		irmo_error_report("irmo_object_set_string",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		
		return;
	}

	if (spec->type != IRMO_TYPE_STRING) {
		irmo_error_report("irmo_object_set_string",
				  "variable '%s' in class '%s' is not string type",
				  variable, object->objclass->name);
		return;
	}

	free(object->variables[spec->index].s);

	object->variables[spec->index].s = strdup(value);

	irmo_object_set_raise(object, spec->index);
}

// get int value

unsigned int irmo_object_get_int(IrmoObject *object, char *variable)
{
	IrmoClassVar *spec;

	irmo_return_val_if_fail(object != NULL, -1);
	irmo_return_val_if_fail(variable != NULL, -1);
	
	spec = irmo_class_get_variable(object->objclass, variable);

	if (!spec) {
		irmo_error_report("irmo_object_get_int",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		
		return -1;
	}

	switch (spec->type) {
	case IRMO_TYPE_INT8:
	case IRMO_TYPE_INT16:
	case IRMO_TYPE_INT32:
		return object->variables[spec->index].i;
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
	IrmoClassVar *spec;

	irmo_return_val_if_fail(object != NULL, NULL);
	irmo_return_val_if_fail(variable != NULL, NULL);

	spec = irmo_class_get_variable(object->objclass, variable);

	if (!spec) {
		irmo_error_report("irmo_object_get_string",
				  "unknown variable '%s' in class '%s'",
				  variable,
				  object->objclass->name);
		
		return NULL;
	}

	if (spec->type != IRMO_TYPE_STRING) {
		irmo_error_report("irmo_object_get_string",
				  "variable '%s' in class '%s' is not a string type",
				  variable, object->objclass->name);
		return NULL;
	}

	return object->variables[spec->index].s;
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
		if (c == klass)
			return 1;
	}

	return 0;
}

unsigned int irmo_object_is_a(IrmoObject *obj, char *classname)
{
	IrmoClass *klass;

	irmo_return_val_if_fail(obj != NULL, 0);
	irmo_return_val_if_fail(classname != NULL, 0);

	klass = irmo_interface_spec_get_class(obj->world->spec, classname);

	if (!klass) {
		irmo_error_report("irmo_object_is_a",
				  "unknown class name '%s'", classname);
		return 0;
	}

	return irmo_object_is_a2(obj, klass);
}

