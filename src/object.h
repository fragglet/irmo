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
	IrmoCallbackData *callbacks;
	ClassSpec *objclass;
	irmo_objid_t id;
	IrmoVariable *variables;
};

// create a new object of a particular class

IrmoObject *object_new(IrmoUniverse *universe, char *typename);

// destroy an object

void object_destroy(IrmoObject *object);

// set the value of an object variable (int type)

void object_set_int(IrmoObject *object, gchar *variable, int value);

// set the value of an object variable (string type)

void object_set_string(IrmoObject *object, gchar *variable, gchar *value);

// get the value of an object variable (int type)

gint object_get_int(IrmoObject *object, gchar *variable);

// get the value of an object variable (string type)

gchar *object_get_string(IrmoObject *object, gchar *variable);


// internal:

void __object_destroy(IrmoObject *object);

#endif /* #ifndef IRMO_OBJECT_H */

// $Log: not supported by cvs2svn $
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/29 14:48:16  sdh300
// variable value retrieval
//
// Revision 1.4  2002/10/21 15:39:36  sdh300
// setting string values
//
// Revision 1.3  2002/10/21 15:10:17  sdh300
// missing cvs tags
//
