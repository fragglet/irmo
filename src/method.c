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
#include "error.h"
#include "method.h"
#include "world.h"

IrmoCallback *irmo_world_method_watch(IrmoWorld *world, 
				      char *method_name,
				      IrmoInvokeCallback method, 
				      void *user_data)
{
	IrmoMethod *spec;

	g_return_val_if_fail(world != NULL, NULL);
	g_return_val_if_fail(method_name != NULL, NULL);
	g_return_val_if_fail(method != NULL, NULL);
	
	spec = irmo_interface_spec_get_method(world->spec, method_name);

	if (!spec) {
		irmo_error_report("irmo_world_method_watch",
				  "unknown method '%s'", method_name);
		return NULL;
	}

	return irmo_callbacklist_add(&world->method_callbacks[spec->index],
				     method, user_data);
}

static void method_invoke_foreach(IrmoCallback *data,
				  IrmoMethodData *method_data)
{
	IrmoInvokeCallback func = (IrmoInvokeCallback) data->func;

	func(method_data, data->user_data);
}

void irmo_method_invoke(IrmoWorld *world, IrmoMethodData *data)
{
	int i;
	
	// check all the arguments for sanity
	
	for (i=0; i<data->spec->narguments; ++i) {
		IrmoMethodArg *arg = data->spec->arguments[i];
		IrmoValue *value = &data->args[i];

		switch (arg->type) {
		case IRMO_TYPE_INT8:
			g_return_if_fail(value->i >= 0 && value->i <= 0xff);
			break;
		case IRMO_TYPE_INT16:
			g_return_if_fail(value->i >= 0 && value->i <= 0xffff);
			break;
		case IRMO_TYPE_INT32:
			break;
		case IRMO_TYPE_STRING:
			g_return_if_fail(value->s != NULL);
			break;
		}
	}
	
	// send to source

	if (world->remote) {
		irmo_client_sendq_add_method(world->remote_client,
					     data);
	}
	
	// invoke callback functions
	
	g_slist_foreach(world->method_callbacks[data->spec->index],
			(GFunc) method_invoke_foreach,
			data);
}

void irmo_world_method_call(IrmoWorld *world, char *method, ...)
{
	IrmoMethodData method_data;
	IrmoMethod *spec;
	IrmoValue *args;
	va_list arglist;
	int i;

	g_return_if_fail(world != NULL);
	g_return_if_fail(method != NULL);
	
	spec = irmo_interface_spec_get_method(world->spec, method);

	if (!spec) {
		irmo_error_report("irmo_world_method_call",
				  "unknown method '%s'", method);
		return;
	}

	args = g_new0(IrmoValue, spec->narguments);

	// read each of the arguments
	
	va_start(arglist, method);
	
	for (i=0; i<spec->narguments; ++i) {
		switch (spec->arguments[i]->type) {
		case IRMO_TYPE_INT8:
		case IRMO_TYPE_INT16:
		case IRMO_TYPE_INT32:
			args[i].i = va_arg(arglist, int);
			break;
		case IRMO_TYPE_STRING:
			args[i].s = va_arg(arglist, char *);
			break;
		}
	}

	va_end(arglist);

	method_data.spec = spec;
	method_data.args = args;
	method_data.src = NULL;

	irmo_method_invoke(world, &method_data);

	free(args);
}


void irmo_world_method_call2(IrmoWorld *world, char *method,
			     IrmoValue *arguments)
{
	IrmoMethodData method_data;
	IrmoMethod *spec;

	g_return_if_fail(world != NULL);
	g_return_if_fail(method != NULL);
	
	spec = irmo_interface_spec_get_method(world->spec, method);

	if (!spec) {
		irmo_error_report("irmo_world_method_call2",
				  "unknown method '%s'", method);
		return;
	}

	method_data.spec = spec;
	method_data.args = arguments;
	method_data.src = NULL;

	irmo_method_invoke(world, &method_data);
}


IrmoClient *irmo_method_get_source(IrmoMethodData *data)
{
	g_return_val_if_fail(data != NULL, NULL);
	
	return data->src;
}

char *irmo_method_arg_string(IrmoMethodData *data, char *argname)
{
	IrmoMethodArg *spec;

	g_return_val_if_fail(data != NULL, NULL);
	g_return_val_if_fail(argname != NULL, NULL);
	
	spec = irmo_method_get_argument(data->spec, argname);

	if (!spec) {
		irmo_error_report("irmo_method_arg_string",
				  "unknown method argument '%s' for '%s' method",
				  data->spec->name, argname);
		return NULL;
	}

	if (spec->type != IRMO_TYPE_STRING) {
		irmo_error_report("irmo_method_arg_string",
				  "'%s' argument for '%s' method is not a string type",
				  argname, data->spec->name);
		return NULL;
	}

	return data->args[spec->index].s;		
}

unsigned int irmo_method_arg_int(IrmoMethodData *data, char *argname)
{
	IrmoMethodArg *spec;

	g_return_val_if_fail(data != NULL, -1);
	g_return_val_if_fail(argname != NULL, -1);
	
	spec = irmo_method_get_argument(data->spec, argname);

	if (!spec) {
		irmo_error_report("irmo_method_arg_int",
				  "unknown method argument '%s' for '%s' method",
				  data->spec->name, argname);
		return 0;
	}

	switch (spec->type) {
	case IRMO_TYPE_INT8:
	case IRMO_TYPE_INT16:
	case IRMO_TYPE_INT32:
		return data->args[spec->index].i;
	default:
		irmo_error_report("irmo_method_arg_int",
				  "'%s' argument for '%s' method is not an integer type",
				  argname, data->spec->name);
		return 0;
	}
}

// $Log$
// Revision 1.12  2003/11/18 18:14:46  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.11  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.10  2003/09/20 15:44:48  fraggle
// Sanity checking on method arguments
//
// Revision 1.9  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.8  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.7  2003/08/29 16:28:19  fraggle
// Iterators for reflection API. Rename IrmoMethodCallback to IrmoInvokeCallback
// to avoid name conflict.
//
// Revision 1.6  2003/08/28 16:43:45  fraggle
// Use the reflection API internally to improve readability in places
//
// Revision 1.5  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.4  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.3  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
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
