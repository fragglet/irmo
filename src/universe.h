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
void universe_ref(IrmoUniverse *universe);
void universe_unref(IrmoUniverse *universe);

#endif /* #ifndef IRMO_UNIVERSE_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
