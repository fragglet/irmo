//
// Object code
//

#include <stdio.h>
#include <stdlib.h>

#include <glib.h>

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
	int i;

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

	// member variables:

	object->variables = malloc(sizeof(IrmoVariable) * spec->nvariables);

	// initialisation

	for (i=0; i<spec->nvariables; ++i)
		object->variables[i].i32 = 0;
	
	// add to universe

	g_hash_table_insert(universe->objects, (gpointer) id, object);

	// TODO: hooks for new object callbacks...
	
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
	
	// done
	
	free(object);
}

void object_destroy(IrmoObject *object)
{
	// remove from universe
	
	g_hash_table_remove(object->universe->objects, (gpointer) object->id);

	// destroy object
	
	__object_destroy(object);
	
	// TODO: hooks for object destruction callbacks
}

// $Log: not supported by cvs2svn $
// Revision 1.1  2002/10/21 14:58:06  sdh300
// split off object code to a seperate module
//
