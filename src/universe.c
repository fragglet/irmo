//
// Irmo Universe
//

#include <stdio.h>

#include "if_spec.h"
#include "universe.h"

IrmoUniverse *universe_new(InterfaceSpec *spec)
{
	IrmoUniverse *universe;

	universe = g_new0(IrmoUniverse, 1);

	universe->spec = spec;
	universe->objects = g_hash_table_new(g_direct_hash, g_direct_equal);
	universe->refcount = 1;
	universe->lastid = 0;

	interface_spec_ref(spec);

	return universe;
}

IrmoObject *universe_get_object_for_id(IrmoUniverse *universe,
				       irmo_objid_t id)
{
	IrmoObject *object;

	object = g_hash_table_lookup(universe->objects, (gpointer) id);

	return object;
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

IrmoObject *universe_object_new(IrmoUniverse *universe, char *typename)
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

	// TODO: member variables
	
	// add to universe

	g_hash_table_insert(universe->objects, (gpointer) id, object);

	// TODO: hooks for new object callbacks...
	
	return object;
}

// $Log: not supported by cvs2svn $
