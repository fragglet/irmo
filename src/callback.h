//
// Callbacks
//

#ifndef IRMO_CALLBACK_H
#define IRMO_CALLBACK_H

typedef struct _IrmoCallbackData IrmoCallbackData;
typedef struct _IrmoCallback IrmoCallback;

#include "if_spec.h"
#include "object.h"

typedef void (*IrmoCallbackFunc) (IrmoObject *object, gchar *variable,
				  gpointer user_data);

struct _IrmoCallback {
	IrmoCallbackFunc func;
	gpointer user_data;
};

struct _IrmoCallbackData {
	ClassSpec *objclass;

	// callbacks for if any variable is changed

	GSList *class_callbacks;

	// callbacks for if a particular variable is changed
	
	GSList **variable_callbacks;
};

IrmoCallbackData *_callbackdata_new(ClassSpec *objclass);
void _callbackdata_free(IrmoCallbackData *data);

void callback_watch_class(IrmoUniverse *universe,
			  gchar *classname, gchar *variable,
			  IrmoCallbackFunc func, gpointer user_data);
void callback_watch_object(IrmoObject *object, gchar *variable,
			   IrmoCallbackFunc func, gpointer user_data);

#endif /* #ifndef IRMO_CALLBACK_H */
