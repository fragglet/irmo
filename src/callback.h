//
// Callbacks
//

#ifndef IRMO_CALLBACK_H
#define IRMO_CALLBACK_H

typedef struct _IrmoCallbackData IrmoCallbackData;
typedef struct _IrmoVarCallbackData IrmoVarCallbackData;

#include "if_spec.h"
#include "object.h"

typedef void (*IrmoCallback) (IrmoObject *object, gchar *variable,
			      gpointer user_data);

struct _IrmoVarCallbackData {
	IrmoCallback func;
	gpointer user_data;
};

struct _IrmoCallbackData {
	ClassSpec *objclass;

	// callbacks for if any variable is changed

	GSList *class_callbacks;

	// callbacks for if a particular variable is changed
	
	GSList **variable_callbacks;

	// callbacks called when object is destroyed
	
	GSList *destroy_callbacks;
};

IrmoCallbackData *_callbackdata_new(ClassSpec *objclass);
void _callbackdata_free(IrmoCallbackData *data);
void _callbackdata_raise(IrmoCallbackData *data,
			 IrmoObject *object, gint variable_index);

void universe_watch_class(IrmoUniverse *universe,
			  gchar *classname, gchar *variable,
			  IrmoCallback func, gpointer user_data);
void object_watch(IrmoObject *object, gchar *variable,
		  IrmoCallback func, gpointer user_data);

#endif /* #ifndef IRMO_CALLBACK_H */

// $Log: not supported by cvs2svn $
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
