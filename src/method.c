// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// Copyright (C) 2002-3 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//---------------------------------------------------------------------

#include "sysheaders.h"

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

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(method_name != NULL, NULL);
	irmo_return_val_if_fail(method != NULL, NULL);
	
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
			irmo_return_if_fail(value->i >= 0 && value->i <= 0xff);
			break;
		case IRMO_TYPE_INT16:
			irmo_return_if_fail(value->i >= 0 && value->i <= 0xffff);
			break;
		case IRMO_TYPE_INT32:
			break;
		case IRMO_TYPE_STRING:
			irmo_return_if_fail(value->s != NULL);
			break;
		}
	}
	
	// send to source

	if (world->remote) {
		irmo_client_sendq_add_method(world->remote_client,
					     data);
	}
	
	// invoke callback functions
	
	irmo_slist_foreach(world->method_callbacks[data->spec->index],
			(IrmoSListIterator) method_invoke_foreach,
			data);
}

void irmo_world_method_call(IrmoWorld *world, char *method, ...)
{
	IrmoMethodData method_data;
	IrmoMethod *spec;
	IrmoValue *args;
	va_list arglist;
	int i;

	irmo_return_if_fail(world != NULL);
	irmo_return_if_fail(method != NULL);
	
	spec = irmo_interface_spec_get_method(world->spec, method);

	if (!spec) {
		irmo_error_report("irmo_world_method_call",
				  "unknown method '%s'", method);
		return;
	}

	args = irmo_new0(IrmoValue, spec->narguments);

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

	irmo_return_if_fail(world != NULL);
	irmo_return_if_fail(method != NULL);
	
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
	irmo_return_val_if_fail(data != NULL, NULL);
	
	return data->src;
}

char *irmo_method_arg_string(IrmoMethodData *data, char *argname)
{
	IrmoMethodArg *spec;

	irmo_return_val_if_fail(data != NULL, NULL);
	irmo_return_val_if_fail(argname != NULL, NULL);
	
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

	irmo_return_val_if_fail(data != NULL, -1);
	irmo_return_val_if_fail(argname != NULL, -1);
	
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

