//
// Irmo Universe
//

#ifndef IRMO_UNIVERSE_H
#define IRMO_UNIVERSE_H

typedef struct _IrmoUniverse IrmoUniverse;
typedef guint irmo_objid_t;

#include <glib.h>
#include "callback.h"
#include "if_spec.h"
#include "object.h"

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoUniverse {
	InterfaceSpec *spec;
	IrmoCallbackData **callbacks;
	GHashTable *objects;
	irmo_objid_t lastid;
	int refcount;
};

// create a new universe from an interface spec

IrmoUniverse *universe_new(InterfaceSpec *spec);

// find an object in the universe

IrmoObject *universe_get_object_for_id(IrmoUniverse *universe,
				       irmo_objid_t id);

// iterate over objects in the universe

void universe_foreach_object(IrmoUniverse *universe, gchar *classname,
			     IrmoObjCallback func, gpointer user_data);

// add reference

void universe_ref(IrmoUniverse *universe);

// remove reference

void universe_unref(IrmoUniverse *universe);

#endif /* #ifndef IRMO_UNIVERSE_H */

// $Log: not supported by cvs2svn $
// Revision 1.7  2002/11/13 13:56:23  sdh300
// add some documentation
//
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/21 14:58:07  sdh300
// split off object code to a seperate module
//
// Revision 1.4  2002/10/21 14:48:54  sdh300
// oops, fix build
//
// Revision 1.3  2002/10/21 14:43:27  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
