#include <stdio.h>
#include <stdlib.h>

#include "callback.h"
#include "method.h"
#include "universe.h"

void irmo_universe_method_watch(IrmoUniverse *universe, gchar *method_name,
				IrmoMethodCallback method, gpointer user_data)
{
	MethodSpec *spec;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(method_name != NULL);
	g_return_if_fail(method != NULL);
	
	spec = g_hash_table_lookup(universe->spec->method_hash,
				   method_name);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_method_watch: Unknown method '%s'\n",
			method_name);
		return;
	}

	irmo_callbacklist_add(&universe->method_callbacks[spec->index],
			      method, user_data);
}

void irmo_universe_method_unwatch(IrmoUniverse *universe, gchar *method_name,
				  IrmoMethodCallback method,
				  gpointer user_data)
{
	MethodSpec *spec;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(method_name != NULL);
	g_return_if_fail(method != NULL);
	
	spec = g_hash_table_lookup(universe->spec->method_hash,
				   method_name);
	
	if (!spec) {
		fprintf(stderr,
			"irmo_universe_method_unwatch: Unknown method '%s'\n",
			method_name);
		return;
	}

	if (!irmo_callbacklist_remove(&universe->method_callbacks[spec->index],
				      method, user_data)) {
		fprintf(stderr,
			"irmo_universe_method_unwatch: watch not found for "
			"'%s' method\n", method_name);
		return;
	}
}

static void method_invoke_foreach(IrmoCallbackFuncData *data,
				  IrmoMethodData *method_data)
{
	IrmoMethodCallback func = (IrmoMethodCallback) data->func;

	func(method_data, data->user_data);
}

void irmo_method_invoke(IrmoUniverse *universe, IrmoMethodData *data)
{
	// send to source

	if (universe->remote) {
		irmo_client_sendq_add_method(universe->remote_client,
					     data);
	}
	
	// invoke callback functions
	
	g_slist_foreach(universe->method_callbacks[data->spec->index],
			(GFunc) method_invoke_foreach,
			data);
}

void irmo_universe_method_call(IrmoUniverse *universe, gchar *method, ...)
{
	IrmoMethodData method_data;
	MethodSpec *spec;
	IrmoVariable *args;
	va_list arglist;
	int i;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(method != NULL);
	
	spec = g_hash_table_lookup(universe->spec->method_hash, method);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_method_call: Unknown method '%s'\n",
			method);
		return;
	}

	args = g_new0(IrmoVariable, spec->narguments);

	// read each of the arguments
	
	va_start(arglist, method);
	
	for (i=0; i<spec->narguments; ++i) {
		switch (spec->arguments[i]->type) {
		case TYPE_INT8:
			args[i].i8 = va_arg(arglist, int);
			break;
		case TYPE_INT16:
			args[i].i16 = va_arg(arglist, int);
			break;
		case TYPE_INT32:
			args[i].i32 = va_arg(arglist, int);
			break;
		case TYPE_STRING:
			args[i].s = va_arg(arglist, char *);
			break;
		}
	}

	va_end(arglist);

	method_data.spec = spec;
	method_data.args = args;
	method_data.src = NULL;

	irmo_method_invoke(universe, &method_data);

	free(args);
}

IrmoClient *irmo_method_get_source(IrmoMethodData *data)
{
	g_return_val_if_fail(data != NULL, NULL);
	
	return data->src;
}

gchar *irmo_method_arg_string(IrmoMethodData *data, gchar *argname)
{
	MethodArgSpec *spec;

	g_return_val_if_fail(data != NULL, NULL);
	g_return_val_if_fail(argname != NULL, NULL);
	
	spec = g_hash_table_lookup(data->spec->argument_hash, argname);

	if (!spec) {
		fprintf(stderr,
			"irmo_method_arg_string: unknown method argument "
			"'%s' for '%s' method\n",
			data->spec->name, argname);
		return NULL;
	}

	if (spec->type != TYPE_STRING) {
		fprintf(stderr,
			"irmo_method_arg_string: '%s' argument for '%s' "
			"method is not a string type\n",
			argname, data->spec->name);
		return NULL;
	}

	return data->args[spec->index].s;		
}

guint irmo_method_arg_int(IrmoMethodData *data, gchar *argname)
{
	MethodArgSpec *spec;

	g_return_val_if_fail(data != NULL, -1);
	g_return_val_if_fail(argname != NULL, -1);
	
	spec = g_hash_table_lookup(data->spec->argument_hash, argname);

	if (!spec) {
		fprintf(stderr,
			"irmo_method_arg_int: unknown method argument "
			"'%s' for '%s' method\n",
			data->spec->name, argname);
		return 0;
	}

	switch (spec->type) {
	case TYPE_INT8:
		return data->args[spec->index].i8;
	case TYPE_INT16:
		return data->args[spec->index].i16;
	case TYPE_INT32:
		return data->args[spec->index].i32;
	default:
		fprintf(stderr,
			"irmo_method_arg_int: '%s' argument for '%s' "
			"method is not an integer type\n",
			argname, data->spec->name);
		return 0;
	}
}

// $Log: not supported by cvs2svn $
// Revision 1.3  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.2  2003/03/16 00:44:19  sdh300
// Pass correct argument to callback functions
//
// Revision 1.1  2003/03/15 02:21:16  sdh300
// Initial method code
//
