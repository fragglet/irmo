//
// Irmo Universe
//

#include <stdio.h>
#include <stdlib.h>

#include "callback.h"
#include "if_spec.h"
#include "universe.h"

IrmoUniverse *universe_new(InterfaceSpec *spec)
{
	IrmoUniverse *universe;
	int i;
	
	universe = g_new0(IrmoUniverse, 1);

	universe->spec = spec;
	universe->objects = g_hash_table_new(g_direct_hash, g_direct_equal);
	universe->refcount = 1;
	universe->lastid = 0;

	interface_spec_ref(spec);

	// create a callback for each class
	
	universe->callbacks = g_new0(IrmoCallbackData *, spec->nclasses);

	for (i=0; i<spec->nclasses; ++i) {
		universe->callbacks[i] = _callbackdata_new(spec->classes[i]);
	}
	
	return universe;
}

void universe_ref(IrmoUniverse *universe)
{
	++universe->refcount;
}

static void universe_unref_foreach(irmo_objid_t id, IrmoObject *object,
				   gpointer user_data)
{
	__object_destroy(object);
}

void universe_unref(IrmoUniverse *universe)
{
	--universe->refcount;

	if (universe->refcount <= 0) {
		int i;

		// delete all objects
		
		g_hash_table_foreach(universe->objects,
				     (GHFunc) universe_unref_foreach, NULL);
		g_hash_table_destroy(universe->objects);

		// delete callbacks
		
		for (i=0; i<universe->spec->nclasses; ++i)
			_callbackdata_free(universe->callbacks[i]);
		
		free(universe->callbacks);

		// no longer using the interface spec
		
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
// Revision 1.6  2002/10/21 15:09:01  sdh300
// object destruction
//
// Revision 1.5  2002/10/21 14:58:07  sdh300
// split off object code to a seperate module
//
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
