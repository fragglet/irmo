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
#include "universe.h"

// do something for all connected clients

typedef void (*ClientCallback) (IrmoClient *client, gpointer user_data);

struct foreach_client_data {
	IrmoUniverse *universe;
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

static void foreach_client(IrmoUniverse *universe,
			   ClientCallback func, gpointer user_data)
{
	struct foreach_client_data data = {
		universe,
		func,
		user_data,
	};
	int i;

	for (i=0; i<universe->servers->len; ++i) {
		IrmoServer *server
			= (IrmoServer *) universe->servers->pdata[i];

		g_hash_table_foreach(server->clients,
				     (GHFunc) foreach_client_foreach,
				     &data);
	}
}		   

static gint get_free_id(IrmoUniverse *universe)
{
	irmo_objid_t start = universe->lastid;

	// keep incrementing until we find one free
	// we increment once to start off with, since the current lastid
	// was assigned to the previous object
	
	do {
		universe->lastid = (universe->lastid + 1) % MAX_OBJECTS;

		// no free spaces
		
		if (universe->lastid == start)
			return -1;
	} while (g_hash_table_lookup(universe->objects,
				     (gpointer) universe->lastid));

	return universe->lastid;
}

IrmoObject *irmo_object_internal_new(IrmoUniverse *universe,
				     ClassSpec *objclass,
				     irmo_objid_t id)
{
	IrmoObject *object;
	int i;
	
	// make object
	
	object = g_new0(IrmoObject, 1);

	object->id = id;
	object->objclass = objclass;
	object->universe = universe;
	object->callbacks = callbackdata_new(objclass);
	
	// member variables:

	object->variables = g_new0(IrmoVariable, objclass->nvariables);

	// int variables will be initialised to 0 by g_new0
	// string values must be initialised to the empty string ("")
	
	for (i=0; i<objclass->nvariables; ++i)
		if (objclass->variables[i]->type == TYPE_STRING)
			object->variables[i].s = g_strdup("");

	
	// add to universe

	g_hash_table_insert(universe->objects, (gpointer) id, object);

	// raise callback functions for new object creation

	callbackdata_raise_new(universe->callbacks[objclass->index],
			       object);

	// notify attached clients

	foreach_client(universe,
		       (ClientCallback) irmo_client_sendq_add_new, object);

	return object;
}
				

IrmoObject *irmo_object_new(IrmoUniverse *universe, char *typename)
{
	ClassSpec *spec;
	gint id;

	g_return_val_if_fail(universe->remote == FALSE, NULL);
	
	spec = g_hash_table_lookup(universe->spec->class_hash, typename);

	if (!spec) {
		fprintf(stderr, "irmo_object_new: unknown type '%s'\n",
			typename);
		return NULL;
	}

	id = get_free_id(universe);

	if (id < 0) {
		fprintf(stderr,
			"irmo_object_new: maximum of %i objects "
			"per universe (no more objects)\n",
			MAX_OBJECTS);
		return NULL;
	}

	return irmo_object_internal_new(universe, spec, id);
}

// internal object destroy function

void irmo_object_internal_destroy(IrmoObject *object,
				  gboolean notify,
				  gboolean remove)
{
	int i;

	if (notify) {
		// raise destroy callbacks
		
		callbackdata_raise_destroy(object->callbacks, object);
		callbackdata_raise_destroy(object->universe->callbacks
					   [object->objclass->index],
					   object);
		
		// notify connected clients
		
		foreach_client(object->universe,
			       (ClientCallback) irmo_client_sendq_add_destroy,
			       object);
	}	

	// remove from universe

	if (remove) {
		g_hash_table_remove(object->universe->objects,
				    (gpointer) object->id);
	}
	
	// destroy member variables

	for (i=0; i<object->objclass->nvariables; ++i) {
		if (object->objclass->variables[i]->type == TYPE_STRING
		    && object->variables[i].s)
			free(object->variables[i].s);
	}

	free(object->variables);
	callbackdata_free(object->callbacks);

	// done
	
	free(object);
}

void irmo_object_destroy(IrmoObject *object)
{
	g_return_if_fail(object->universe->remote == FALSE);
	
	// destroy object
	// notify callbacks and remove from universe
	
	irmo_object_internal_destroy(object, TRUE, TRUE);
}

irmo_objid_t irmo_object_get_id(IrmoObject *object)
{
	return object->id;
}

gchar *irmo_object_get_class(IrmoObject *object)
{
	return object->objclass->name;
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
	ClassSpec *objclass = object->objclass;
	ClassVarSpec *spec = objclass->variables[variable];
	struct set_notify_data data = {
		object,
		variable,
	};

	// call callback functions for change

	callbackdata_raise(object->callbacks, object, spec->index);
	callbackdata_raise(object->universe->callbacks[objclass->index],
			   object, variable);
	
	// notify clients

	foreach_client(object->universe,
		       (ClientCallback) object_set_notify_foreach,
		       &data);
}

void irmo_object_set_int(IrmoObject *object, gchar *variable, gint value)
{
	ClassVarSpec *spec;
	
	g_return_if_fail(object->universe->remote == FALSE);
	
	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"irmo_object_set_int: unknown variable '%s' "
			"in class '%s'\n",
			variable,
			object->objclass->name);
		
		return;
	}

	switch (spec->type) {
	case TYPE_INT8:
		object->variables[spec->index].i8 = value;
		break;
	case TYPE_INT16:
		object->variables[spec->index].i16 = value;
		break;
	case TYPE_INT32:
		object->variables[spec->index].i32 = value;
		break;
	default:
		fprintf(stderr,
			"irmo_object_set_int: variable '%s' in class '%s' "
			"is not an int type\n",
			variable, object->objclass->name);
		return;
	}

	irmo_object_set_raise(object, spec->index);
}

void irmo_object_set_string(IrmoObject *object, gchar *variable, gchar *value)
{
	ClassVarSpec *spec;

	g_return_if_fail(object->universe->remote == FALSE);
	g_return_if_fail(value != NULL);
	
	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"irmo_object_set_string: unknown variable '%s' "
			"in class '%s'\n",
			variable,
			object->objclass->name);
		
		return;
	}

	if (spec->type != TYPE_STRING) {
		fprintf(stderr,
			"irmo_object_set_string: variable '%s' in class '%s' "
			"is not string type\n",
			variable, object->objclass->name);
		return;
	}

	free(object->variables[spec->index].s);

	object->variables[spec->index].s = strdup(value);

	irmo_object_set_raise(object, spec->index);
}

// get int value

gint irmo_object_get_int(IrmoObject *object, gchar *variable)
{
	ClassVarSpec *spec;

	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"irmo_object_get_int: unknown variable '%s' "
			"in class '%s'\n",
			variable,
			object->objclass->name);
		
		return -1;
	}

	switch (spec->type) {
	case TYPE_INT8:
		return object->variables[spec->index].i8;
	case TYPE_INT16:
		return object->variables[spec->index].i16;
	case TYPE_INT32:
		return object->variables[spec->index].i32;
	default:
		fprintf(stderr,
			"irmo_object_get_int: variable '%s' in class '%s' "
			"is not an int type\n",
			variable, object->objclass->name);
		return -1;
	}
}

// get int value

gchar *irmo_object_get_string(IrmoObject *object, gchar *variable)
{
	ClassVarSpec *spec;

	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"irmo_object_get_string: unknown variable '%s' "
			"in class '%s'\n",
			variable,
			object->objclass->name);
		
		return NULL;
	}

	if (spec->type != TYPE_STRING) {
		fprintf(stderr,
			"irmo_object_get_string: variable '%s' in class '%s' "
			"is not a string type\n",
			variable, object->objclass->name);
		return NULL;
	}

	return object->variables[spec->index].s;
}

// $Log: not supported by cvs2svn $
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
