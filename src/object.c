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
// Object code
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "callback.h"
#include "client.h"
#include "object.h"
#include "sendatom.h"
#include "world.h"

// do something for all connected clients

typedef void (*ClientCallback) (IrmoClient *client, gpointer user_data);

struct foreach_client_data {
	IrmoWorld *world;
	ClientCallback func;
	gpointer user_data;
};

static void foreach_client_foreach(gpointer key, IrmoClient *client,
				   struct foreach_client_data *data)
{
	// only do this for clients which are full connected

	if (client->state != CLIENT_CONNECTED)
		return;

	data->func(client, data->user_data);
}

static void foreach_client(IrmoWorld *world,
			   ClientCallback func, gpointer user_data)
{
	struct foreach_client_data data = {
		world,
		func,
		user_data,
	};
	int i;

	for (i=0; i<world->servers->len; ++i) {
		IrmoServer *server
			= (IrmoServer *) world->servers->pdata[i];

		g_hash_table_foreach(server->clients,
				     (GHFunc) foreach_client_foreach,
				     &data);
	}
}		   

static gint get_free_id(IrmoWorld *world)
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
	} while (g_hash_table_lookup(world->objects,
				     (gpointer) world->lastid));

	return world->lastid;
}

IrmoObject *irmo_object_internal_new(IrmoWorld *world,
				     IrmoClass *objclass,
				     IrmoObjectID id)
{
	IrmoObject *object;
	int i;

	// make object
	
	object = g_new0(IrmoObject, 1);

	object->id = id;
	object->objclass = objclass;
	object->world = world;
	object->callbacks = irmo_callbackdata_new(objclass, NULL);
	
	// member variables:

	object->variables = g_new0(IrmoValue, objclass->nvariables);

	// int variables will be initialised to 0 by g_new0
	// string values must be initialised to the empty string ("")
	
	for (i=0; i<objclass->nvariables; ++i)
		if (objclass->variables[i]->type == IRMO_TYPE_STRING)
			object->variables[i].s = g_strdup("");

	
	// add to world

	g_hash_table_insert(world->objects, (gpointer) id, object);

	// raise callback functions for new object creation

	irmo_callbackdata_raise_new(world->callbacks[objclass->index],
				    object);
	irmo_callbackdata_raise_new(world->callbacks_all, object);

	// notify attached clients

	foreach_client(world,
		       (ClientCallback) irmo_client_sendq_add_new, object);

	// if a remote world, create variable_time array

	if (world->remote)
		object->variable_time = g_new0(int, objclass->nvariables);
	
	return object;
}
				

IrmoObject *irmo_object_new(IrmoWorld *world, char *typename)
{
	IrmoClass *spec;
	gint id;

	g_return_val_if_fail(world != NULL, NULL);
	g_return_val_if_fail(typename != NULL, NULL);
	g_return_val_if_fail(world->remote == FALSE, NULL);
	
	spec = irmo_interface_spec_get_class(world->spec, typename);

	if (!spec) {
		irmo_error_report("irmo_object_new", 
				  "unknown type '%s'", typename);
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
				  gboolean notify,
				  gboolean remove)
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
		g_hash_table_remove(object->world->objects,
				    (gpointer) object->id);
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
	g_return_if_fail(object != NULL);
	g_return_if_fail(object->world->remote == FALSE);
	
	// destroy object
	// notify callbacks and remove from world
	
	irmo_object_internal_destroy(object, TRUE, TRUE);
}

IrmoObjectID irmo_object_get_id(IrmoObject *object)
{
	g_return_val_if_fail(object != NULL, -1);
	
	return object->id;
}

char *irmo_object_get_class(IrmoObject *object)
{
	g_return_val_if_fail(object != NULL, NULL);
	
	return object->objclass->name;
}

IrmoClass *irmo_object_get_class_obj(IrmoObject *object)
{
	g_return_val_if_fail(object != NULL, NULL);

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

	g_return_if_fail(object != NULL);
	g_return_if_fail(variable != NULL);
	g_return_if_fail(object->world->remote == FALSE);
	
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
		g_return_if_fail(value >= 0 && value <= 0xff);
		obj_var->i = value;
		break;
	case IRMO_TYPE_INT16:
		g_return_if_fail(value >= 0 && value <= 0xffff);
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

	g_return_if_fail(object != NULL);
	g_return_if_fail(variable != NULL);
	g_return_if_fail(value != NULL);
	g_return_if_fail(object->world->remote == FALSE);
	
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

	g_return_val_if_fail(object != NULL, -1);
	g_return_val_if_fail(variable != NULL, -1);
	
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

	g_return_val_if_fail(object != NULL, NULL);
	g_return_val_if_fail(variable != NULL, NULL);

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
	g_return_val_if_fail(obj != NULL, NULL);

	return obj->world;
}

unsigned int irmo_object_is_a2(IrmoObject *obj, IrmoClass *klass)
{
	IrmoClass *c;
	
	g_return_val_if_fail(obj != NULL, FALSE);
	g_return_val_if_fail(klass != NULL, FALSE);

	// search through all parent classes

	for (c=obj->objclass; c; c=c->parent_class) {
		if (c == klass)
			return TRUE;
	}

	return FALSE;
}

unsigned int irmo_object_is_a(IrmoObject *obj, char *classname)
{
	IrmoClass *klass;

	g_return_val_if_fail(obj != NULL, FALSE);
	g_return_val_if_fail(classname != NULL, FALSE);

	klass = irmo_interface_spec_get_class(obj->world->spec, classname);

	if (!klass) {
		irmo_error_report("irmo_object_is_a",
				  "unknown class name '%s'", classname);
		return FALSE;
	}

	return irmo_object_is_a2(obj, klass);
}

// $Log$
// Revision 1.15  2003/11/17 00:32:27  fraggle
// Rename irmo_objid_t to IrmoObjectID for consistency with other types
//
// Revision 1.14  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.13  2003/09/13 16:11:48  fraggle
// Guard against overflows when setting int values
//
// Revision 1.12  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.11  2003/09/02 20:33:55  fraggle
// Subclassing in interfaces
//
// Revision 1.10  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.9  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.8  2003/08/31 18:20:32  fraggle
// irmo_object_get_class_obj
//
// Revision 1.7  2003/08/28 16:43:45  fraggle
// Use the reflection API internally to improve readability in places
//
// Revision 1.6  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.5  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
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
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.24  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.23  2003/05/20 02:06:06  sdh300
// Add out-of-order execution of stream
//
// Revision 1.22  2003/04/25 01:26:19  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.21  2003/03/12 18:58:24  sdh300
// Only send changes to clients which are properly connected.
// This fixes problems sending changes to clients not yet completely
// connected.
//
// Revision 1.20  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.19  2003/03/06 21:28:33  sdh300
// String variables in objects always point to a string; they begin as
// the empty string ("") and not NULL as before
//
// Revision 1.18  2003/03/06 20:53:16  sdh300
// Checking of remote flag for universe objects
//
// Revision 1.17  2003/03/06 19:21:25  sdh300
// Split off some of the constructor/destructor/change code into
// seperate functions that can be reused elsewhere
//
// Revision 1.16  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.15  2003/02/18 20:26:42  sdh300
// Initial send queue building/notification code
//
// Revision 1.14  2002/11/13 15:12:33  sdh300
// object_get_id to get identifier
//
// Revision 1.13  2002/11/13 13:57:41  sdh300
// object_get_class to get the class of an object
//
// Revision 1.12  2002/11/12 23:26:38  sdh300
// this is simpler and clearer
//
// Revision 1.11  2002/11/05 16:28:10  sdh300
// new object callbacks
//
// Revision 1.10  2002/11/05 16:00:37  sdh300
// various "oops"'es
//
// Revision 1.9  2002/11/05 15:55:13  sdh300
// object destroy callbacks
//
// Revision 1.8  2002/10/29 16:28:50  sdh300
// functioning callbacks
//
// Revision 1.7  2002/10/29 16:09:10  sdh300
// initial callback code
//
// Revision 1.6  2002/10/29 15:58:25  sdh300
// add index entry for ClassSpec, use "index" instead of n for index
// member name
//
// Revision 1.5  2002/10/29 14:48:16  sdh300
// variable value retrieval
//
// Revision 1.4  2002/10/21 15:39:35  sdh300
// setting string values
//
// Revision 1.3  2002/10/21 15:32:35  sdh300
// variable value setting
//
// Revision 1.2  2002/10/21 15:09:01  sdh300
// object destruction
//
// Revision 1.1  2002/10/21 14:58:06  sdh300
// split off object code to a seperate module
//
