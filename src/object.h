//
// Irmo Objects
//

#ifndef IRMO_INTERNAL_OBJECT_H
#define IRMO_INTERNAL_OBJECT_H

#include "public/object.h"

#include "callback.h"

typedef union _IrmoVariable IrmoVariable;

// internal stuff:

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

void object_internal_destroy(IrmoObject *object);

#endif /* #ifndef IRMO_OBJECT_H */

// $Log: not supported by cvs2svn $
// Revision 1.12  2003/02/23 00:00:03  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.11  2002/11/17 20:37:15  sdh300
// initial doxygen documentation
//
// Revision 1.10  2002/11/13 15:12:34  sdh300
// object_get_id to get identifier
//
// Revision 1.9  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.8  2002/11/13 13:57:42  sdh300
// object_get_class to get the class of an object
//
// Revision 1.7  2002/11/13 13:56:24  sdh300
// add some documentation
//
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
