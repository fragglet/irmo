//
// Irmo Universe
//

#ifndef IRMO_UNIVERSE_H
#define IRMO_UNIVERSE_H

typedef struct _IrmoUniverse IrmoUniverse;
typedef struct _IrmoObject IrmoObject;
typedef guint irmo_objid_t;

#include <glib.h>
#include "if_spec.h"

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoObject {
	IrmoUniverse *universe;
	ClassSpec *objclass;
	irmo_objid_t id;
};

struct _IrmoUniverse {
	InterfaceSpec *spec;
	GHashTable *objects;
	irmo_objid_t lastid;
	int refcount;
};

IrmoUniverse *universe_new(InterfaceSpec *spec);
IrmoObject *universe_get_object_for_id(IrmoUniverse *universe,
				       irmo_objid_t id);
IrmoObject *universe_object_new(IrmoUniverse *universe, char *typename);

#endif /* #ifndef IRMO_UNIVERSE_H */

// $Log: not supported by cvs2svn $
