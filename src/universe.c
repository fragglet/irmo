//
// Irmo Universe
//

#include <stdio.h>
#include <stdlib.h>

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

void universe_ref(IrmoUniverse *universe)
{
	++universe->refcount;
}

static void object_destroy(irmo_objid_t id, IrmoObject *object,
			   gpointer user_data)
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

void universe_unref(IrmoUniverse *universe)
{
	--universe->refcount;

	if (universe->refcount <= 0) {
		g_hash_table_foreach(universe->objects,
				     (GHFunc) object_destroy, NULL);
		g_hash_table_destroy(universe->objects);
		interface_spec_unref(universe->spec);
		free(universe);
	}
}

IrmoObject *universe_get_object_for_id(IrmoUniverse *universe,
				       irmo_objid_t id)
{
	IrmoObject *object;

	object = g_hash_table_lookup(universe->objects, (gpointer) id);

	return object;
}

// $Log: not supported by cvs2svn $
// Revision 1.4  2002/10/21 14:48:54  sdh300
// oops, fix build
//
// Revision 1.3  2002/10/21 14:43:26  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
