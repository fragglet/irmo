// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>

#include "callback.h"
#include "method.h"
#include "universe.h"

IrmoCallback *irmo_universe_method_watch(IrmoUniverse *universe, 
					 gchar *method_name,
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
			"irmo_universe_method_watch: Unknown method '%s'\n",
			method_name);
		return;
	}

	return irmo_callbacklist_add(&universe->method_callbacks[spec->index],
				     method, user_data);
}

static void method_invoke_foreach(IrmoCallback *data,
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

// $Log$
// Revision 1.2  2003/07/22 02:05:39  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.5  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.4  2003/04/25 01:26:19  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.3  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.2  2003/03/16 00:44:19  sdh300
// Pass correct argument to callback functions
//
// Revision 1.1  2003/03/15 02:21:16  sdh300
// Initial method code
//
