//
// Object code
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <glib.h>

#include "callback.h"
#include "object.h"
#include "universe.h"

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

IrmoObject *object_new(IrmoUniverse *universe, char *typename)
{
	IrmoObject *object;
	ClassSpec *spec;
	gint id;

	spec = g_hash_table_lookup(universe->spec->class_hash, typename);

	if (!spec) {
		fprintf(stderr, "universe_object_new: unknown type '%s'\n",
			typename);
		return NULL;
	}

	id = get_free_id(universe);

	if (id < 0) {
		fprintf(stderr,
			"universe_object_new: maximum of %i objects "
			"per universe (no more objects)\n",
			MAX_OBJECTS);
		return NULL;
	}

	// make object
	
	object = g_new0(IrmoObject, 1);

	object->id = id;
	object->objclass = spec;
	object->universe = universe;
	object->callbacks = _callbackdata_new(spec);
	
	// member variables:

	object->variables = g_new0(IrmoVariable, spec->nvariables);
	
	// add to universe

	g_hash_table_insert(universe->objects, (gpointer) id, object);

	// raise callback functions for new object creation

	_callbackdata_raise_new(universe->callbacks[spec->index],
				object);
	
	return object;
}

// internal object destroy function

void __object_destroy(IrmoObject *object)
{
	int i;

	// destroy member variables

	for (i=0; i<object->objclass->nvariables; ++i) {
		if (object->objclass->variables[i]->type == TYPE_STRING
		    && object->variables[i].s)
			free(object->variables[i].s);
	}

	free(object->variables);
	_callbackdata_free(object->callbacks);
	
	// done
	
	free(object);
}

void object_destroy(IrmoObject *object)
{
	// raise destroy callbacks

	_callbackdata_raise_destroy(object->callbacks, object);
	_callbackdata_raise_destroy(object->universe->callbacks
				    [object->objclass->index],
				    object);

	// remove from universe
	
	g_hash_table_remove(object->universe->objects, (gpointer) object->id);

	// destroy object
	
	__object_destroy(object);
}

irmo_objid_t object_get_id(IrmoObject *object)
{
	return object->id;
}

gchar *object_get_class(IrmoObject *object)
{
	return object->objclass->name;
}

void object_set_int(IrmoObject *object, gchar *variable, gint value)
{
	ClassVarSpec *spec;

	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"object_set_int: unknown variable '%s' "
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
			"object_set_int: variable '%s' in class '%s' "
			"is not an int type\n",
			variable, object->objclass->name);
		return;
	}

	// callback functions for change

	_callbackdata_raise(object->callbacks, object, spec->index);
	_callbackdata_raise(object->universe->callbacks[object->objclass->index],
			    object, spec->index);
}

void object_set_string(IrmoObject *object, gchar *variable, gchar *value)
{
	ClassVarSpec *spec;

	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"object_set_string: unknown variable '%s' "
			"in class '%s'\n",
			variable,
			object->objclass->name);
		
		return;
	}

	if (spec->type != TYPE_STRING) {
		fprintf(stderr,
			"object_set_string: variable '%s' in class '%s' "
			"is not string type\n",
			variable, object->objclass->name);
		return;
	}

	if (object->variables[spec->index].s)
		free(object->variables[spec->index].s);

	object->variables[spec->index].s = strdup(value);

	// callback functions

	_callbackdata_raise(object->callbacks, object, spec->index);
	_callbackdata_raise(object->universe->callbacks[object->objclass->index],
			    object, spec->index);
}

// get int value

gint object_get_int(IrmoObject *object, gchar *variable)
{
	ClassVarSpec *spec;

	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"object_get_int: unknown variable '%s' "
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
			"object_get_int: variable '%s' in class '%s' "
			"is not an int type\n",
			variable, object->objclass->name);
		return -1;
	}
}

// get int value

gchar *object_get_string(IrmoObject *object, gchar *variable)
{
	ClassVarSpec *spec;

	spec = g_hash_table_lookup(object->objclass->variable_hash,
				   variable);

	if (!spec) {
		fprintf(stderr,
			"object_get_string: unknown variable '%s' "
			"in class '%s'\n",
			variable,
			object->objclass->name);
		
		return NULL;
	}

	if (spec->type != TYPE_STRING) {
		fprintf(stderr,
			"object_get_string: variable '%s' in class '%s' "
			"is not a string type\n",
			variable, object->objclass->name);
		return NULL;
	}

	return object->variables[spec->index].s;
}

// $Log: not supported by cvs2svn $
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
