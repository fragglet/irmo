//
// Irmo Objects
//

#ifndef IRMO_OBJECT_H
#define IRMO_OBJECT_H

typedef struct _IrmoObject IrmoObject;
typedef union _IrmoVariable IrmoVariable;

#include "universe.h"

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

IrmoObject *object_new(IrmoUniverse *universe, char *typename);


#endif /* #ifndef IRMO_OBJECT_H */
