
#include <stdio.h>

#include "callback.h"
#include "if_spec.h"

IrmoCallbackData *_callbackdata_new(ClassSpec *objclass)
{
	IrmoCallbackData *data;

	data = g_new0(IrmoCallbackData, 1);

	data->objclass = objclass;

	data->class_callbacks = NULL;
	data->variable_callbacks = g_new0(GSList *, objclass->nvariables);

	return data;
}

static void _callbackdata_free_foreach(IrmoCallback *callback,
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

void _callbackdata_raise_foreach(IrmoCallback *callback,
				 struct raise_data *raise_data)
{
	callback->func(raise_data->object, raise_data->variable,
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

static void callbackdata_watch(IrmoCallbackData *data,
			       gchar *variable,
			       IrmoCallbackFunc func, gpointer user_data)
{
	GSList **list;
	IrmoCallback *callback;
	
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

	callback = g_new0(IrmoCallback, 1);
	callback->func = func;
	callback->user_data = user_data;

	*list = g_slist_append(*list, callback);
}

void callback_watch_class(IrmoUniverse *universe,
			  gchar *classname, gchar *variable,
			  IrmoCallbackFunc func, gpointer user_data)
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

void callback_watch_object(IrmoObject *object, gchar *variable,
			   IrmoCallbackFunc func, gpointer user_data)
{
	callbackdata_watch(object->callbacks, variable,
			   func, user_data);
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2002/10/29 16:10:19  sdh300
// add missing cvs tags
//
