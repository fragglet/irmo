//
// Callbacks
//

#ifndef IRMO_INTERNAL_CALLBACK_H
#define IRMO_INTERNAL_CALLBACK_H

typedef struct _IrmoCallbackData IrmoCallbackData;
typedef struct _IrmoCallbackFuncData IrmoCallbackFuncData;

#include "public/callback.h"

#include "if_spec.h"
#include "object.h"

struct _IrmoCallbackFuncData {
	union {
		IrmoVarCallback var;
		IrmoObjCallback obj;
	} func;
	gpointer user_data;
};

struct _IrmoCallbackData {
	ClassSpec *objclass;

	// callbacks for watching for creations of new objects
	// this is only for the class callbackdata - redundant
	// in object callbacks

	GSList *new_callbacks;
	
	// callbacks for if any variable is changed

	GSList *class_callbacks;

	// callbacks for if a particular variable is changed
	
	GSList **variable_callbacks;

	// callbacks called when object is destroyed
	
	GSList *destroy_callbacks;
};

IrmoCallbackData *callbackdata_new(ClassSpec *objclass);
void callbackdata_free(IrmoCallbackData *data);
void callbackdata_raise(IrmoCallbackData *data,
			 IrmoObject *object, gint variable_index);
void callbackdata_raise_destroy(IrmoCallbackData *data, IrmoObject *object);
void callbackdata_raise_new(IrmoCallbackData *data, IrmoObject *object);

#endif /* #ifndef IRMO_INTERNAL_CALLBACK_H */

// $Log: not supported by cvs2svn $
// Revision 1.12  2003/02/23 00:00:03  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.11  2002/11/17 20:37:14  sdh300
// initial doxygen documentation
//
// Revision 1.10  2002/11/13 14:14:45  sdh300
// object iterator function
//
// Revision 1.9  2002/11/12 23:04:31  sdh300
// callback removal, and code cleanup/generalisation
//
// Revision 1.8  2002/11/05 22:33:26  sdh300
// more name changes
//
// Revision 1.7  2002/11/05 16:28:10  sdh300
// new object callbacks
//
// Revision 1.6  2002/11/05 15:55:12  sdh300
// object destroy callbacks
//
// Revision 1.5  2002/11/05 15:17:17  sdh300
// more consistent naming for callback types
//
// Revision 1.4  2002/11/05 15:01:06  sdh300
// change callback function names
// initial destroy callback variables
//
// Revision 1.3  2002/10/29 16:28:50  sdh300
// functioning callbacks
//
// Revision 1.2  2002/10/29 16:10:19  sdh300
// add missing cvs tags
//
