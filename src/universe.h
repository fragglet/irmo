//
// Irmo Universe
//

#ifndef IRMO_UNIVERSE_H
#define IRMO_UNIVERSE_H

typedef struct _IrmoUniverse IrmoUniverse;
typedef guint irmo_objid_t;

#include <glib.h>
#include "if_spec.h"
#include "object.h"

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoUniverse {
	InterfaceSpec *spec;
	GHashTable *objects;
	irmo_objid_t lastid;
	int refcount;
};

IrmoUniverse *universe_new(InterfaceSpec *spec);
IrmoObject *universe_get_object_for_id(IrmoUniverse *universe,
				       irmo_objid_t id);
void universe_ref(IrmoUniverse *universe);
void universe_unref(IrmoUniverse *universe);

#endif /* #ifndef IRMO_UNIVERSE_H */

// $Log: not supported by cvs2svn $
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
