
#include <stdio.h>
#include <stdlib.h>

#include "callback.h"
#include "if_spec.h"

IrmoCallbackData *_callbackdata_new(ClassSpec *objclass)
{
	IrmoCallbackData *data;

	data = g_new0(IrmoCallbackData, 1);

	data->objclass = objclass;

	data->class_callbacks = NULL;
	data->destroy_callbacks = NULL;
	data->variable_callbacks = g_new0(GSList *, objclass->nvariables);

	return data;
}

static void _callbackdata_free_foreach(IrmoVarCallbackData *callback,
				       gpointer user_data)
{
	free(callback);
}

void _callbackdata_free(IrmoCallbackData *data)
{
	int i;
	
	// free all class callbacks

	g_slist_foreach(data->class_callbacks,
			(GFunc) _callbackdata_free_foreach, NULL);
	g_slist_free(data->class_callbacks);

	// free destroy callbacks

	g_slist_foreach(data->destroy_callbacks,
			(GFunc) _callbackdata_free_foreach, NULL);
	g_slist_free(data->destroy_callbacks);
	
	// free all variable callbacks

	for (i=0; i<data->objclass->nvariables; ++i) {
		g_slist_foreach(data->variable_callbacks[i],
				(GFunc) _callbackdata_free_foreach, NULL);
		g_slist_free(data->variable_callbacks[i]);
	}

	free(data->variable_callbacks);
	
	free(data);
}

struct raise_data {
	IrmoObject *object;
	gchar *variable;
};

void _callbackdata_raise_foreach(IrmoVarCallbackData *callback, 
				 struct raise_data *raise_data)
{
	callback->func.var(raise_data->object, raise_data->variable,
			   callback->user_data);
}

void _callbackdata_raise(IrmoCallbackData *data,
			 IrmoObject *object, gint variable_index)
{
	struct raise_data raise_data = {
		object: object,
		variable: data->objclass->variables[variable_index]->name,
	};

	// call class callbacks
	
	g_slist_foreach(data->class_callbacks,
			(GFunc) _callbackdata_raise_foreach,
			&raise_data);

	// variable callbacks

	g_slist_foreach(data->variable_callbacks[variable_index],
			(GFunc) _callbackdata_raise_foreach,
			&raise_data);
}

static void _callbackdata_raise_destroy_foreach(IrmoVarCallbackData *callback,
						struct raise_data *raise_data)
{
	callback->func.destroy(raise_data->object, callback->user_data);
}

void _callbackdata_raise_destroy(IrmoCallbackData *data,
				 IrmoObject *object)
{
	struct raise_data raise_data = {
		object: object,
	};

	g_slist_foreach(data->destroy_callbacks,
			(GFunc) _callbackdata_raise_destroy_foreach,
			&raise_data);
}

static void callbackdata_watch(IrmoCallbackData *data,
			       gchar *variable,
			       IrmoCallback func, gpointer user_data)
{
	GSList **list;
	IrmoVarCallbackData *callback;
	
	if (variable) {
		ClassVarSpec *varspec
			= g_hash_table_lookup(data->objclass->variable_hash,
					      variable);

		if (!varspec) {
			fprintf(stderr,
				"callbackdata_watch: unknown variable '%s' "
				"in class '%s'\n",
				data->objclass->name, variable);
			return;
		}

		list = &data->variable_callbacks[varspec->index];
	} else {
		list = &data->class_callbacks;
	}

	callback = g_new0(IrmoVarCallbackData, 1);
	callback->func.var = func;
	callback->user_data = user_data;

	*list = g_slist_append(*list, callback);
}

static void callbackdata_watch_destroy(IrmoCallbackData *data,
				       IrmoDestroyCallback func,
				       gpointer user_data)
{
	IrmoVarCallbackData *callback;
	
	callback = g_new0(IrmoVarCallbackData, 1);
	callback->func.destroy = func;
	callback->user_data = user_data;

	data->destroy_callbacks = g_slist_append(data->destroy_callbacks,
						 callback);
}

void universe_watch_class(IrmoUniverse *universe,
			  gchar *classname, gchar *variable,
			  IrmoCallback func, gpointer user_data)
{
	ClassSpec *spec;
	
	// find the class
	
	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"callback_watch_class: unknown class '%s'\n",
			classname);
		return;
	}

	callbackdata_watch(universe->callbacks[spec->index], variable,
			   func, user_data);
}

void universe_watch_destroy(IrmoUniverse *universe, gchar *classname,
			    IrmoDestroyCallback func, gpointer user_data)
{
	ClassSpec *spec;

	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"universe_watch_destroy: unknown class '%s'\n",
			classname);
		return;
	}

	callbackdata_watch_destroy(universe->callbacks[spec->index],
				   func, user_data);
}

void object_watch(IrmoObject *object, gchar *variable,
		  IrmoCallback func, gpointer user_data)
{
	callbackdata_watch(object->callbacks, variable,
			   func, user_data);
}

void object_watch_destroy(IrmoObject *object,
			  IrmoDestroyCallback func, gpointer user_data)
{
	callbackdata_watch_destroy(object->callbacks,
				   func, user_data);
}

// $Log: not supported by cvs2svn $
// Revision 1.6  2002/11/05 15:17:17  sdh300
// more consistent naming for callback types
//
// Revision 1.5  2002/11/05 15:04:11  sdh300
// more warnings!
//
// Revision 1.4  2002/11/05 15:01:05  sdh300
// change callback function names
// initial destroy callback variables
//
// Revision 1.3  2002/10/29 16:28:50  sdh300
// functioning callbacks
//
// Revision 1.2  2002/10/29 16:10:19  sdh300
// add missing cvs tags
//
