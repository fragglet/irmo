//
// Callbacks
//

#ifndef IRMO_CALLBACK_H
#define IRMO_CALLBACK_H

typedef struct _IrmoCallbackData IrmoCallbackData;
typedef struct _IrmoCallbackFuncData IrmoCallbackFuncData;

#include "if_spec.h"
#include "object.h"

typedef void (*IrmoVarCallback) (IrmoObject *object, gchar *variable,
				 gpointer user_data);
typedef void (*IrmoObjCallback) (IrmoObject *object, gpointer user_data);

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

IrmoCallbackData *_callbackdata_new(ClassSpec *objclass);
void _callbackdata_free(IrmoCallbackData *data);
void _callbackdata_raise(IrmoCallbackData *data,
			 IrmoObject *object, gint variable_index);
void _callbackdata_raise_destroy(IrmoCallbackData *data, IrmoObject *object);
void _callbackdata_raise_new(IrmoCallbackData *data, IrmoObject *object);

// watch creation of new objects of particular classes

void universe_watch_new(IrmoUniverse *universe, gchar *classname,
			IrmoObjCallback func, gpointer user_data);
void universe_unwatch_new(IrmoUniverse *universe, gchar *classname,
			  IrmoObjCallback func, gpointer user_data);

// watch variables of a particular class

void universe_watch_class(IrmoUniverse *universe,
			  gchar *classname, gchar *variable,
			  IrmoVarCallback func, gpointer user_data);
void universe_unwatch_class(IrmoUniverse *universe,
			    gchar *classname, gchar *variable,
			    IrmoVarCallback func, gpointer user_data);

// watch object destruction

void universe_watch_destroy(IrmoUniverse *universe, gchar *classname,
			    IrmoObjCallback func, gpointer user_data);
void universe_unwatch_destroy(IrmoUniverse *universe, gchar *classname,
			      IrmoObjCallback func, gpointer user_data);

// watch variables of a particular object

void object_watch(IrmoObject *object, gchar *variable,
		  IrmoVarCallback func, gpointer user_data);
void object_unwatch(IrmoObject *object, gchar *variable,
		    IrmoVarCallback func, gpointer user_data);

// watch destruction of a particular object

void object_watch_destroy(IrmoObject *object,
			  IrmoObjCallback func, gpointer user_data);
void object_unwatch_destroy(IrmoObject *object,
			    IrmoObjCallback func, gpointer user_data);

#endif /* #ifndef IRMO_CALLBACK_H */

// $Log: not supported by cvs2svn $
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
