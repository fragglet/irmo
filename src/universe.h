//
// Irmo Universe
//

#ifndef IRMO_UNIVERSE_H
#define IRMO_UNIVERSE_H

typedef struct _IrmoUniverse IrmoUniverse;
typedef struct _IrmoObject IrmoObject;
typedef union _IrmoVariable IrmoVariable;
typedef guint irmo_objid_t;

#include <glib.h>
#include "if_spec.h"

// imposed by network protocol:

#define MAX_OBJECTS 65536 

union _IrmoVariable {
	guint8 i8;
	guint16 i16;
	guint32 i32;
	gchar *s;
};

struct _IrmoObject {
	IrmoUniverse *universe;
	ClassSpec *objclass;
	irmo_objid_t id;
	IrmoVariable *variables;
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
// Revision 1.3  2002/10/21 14:43:27  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
