//
// Copyright (C) 2002-2008 Simon Howard
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

#include "arch/sysheaders.h"
#include "base/alloc.h"
#include "base/assert.h"

#include "net/server-world.h"

#include "method.h"
#include "world.h"

IrmoCallback *irmo_world_method_watch(IrmoWorld *world, 
				      char *method_name,
				      IrmoInvokeCallback callback, 
				      void *user_data)
{
	IrmoMethod *method;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(method_name != NULL, NULL);
	irmo_return_val_if_fail(callback != NULL, NULL);
	
	method = irmo_interface_get_method(world->iface, method_name);

	if (method == NULL) {
                irmo_warning_message("irmo_world_method_watch",
                                     "unknown method '%s'", method_name);
                return NULL;
	}

	return irmo_callback_list_add(&world->method_callbacks[method->index],
                                      callback, user_data);
}

// Go through a list of IrmoInvokeCallback callback functions and invoke
// them.

static void invoke_method_callbacks(IrmoCallbackList *list,
                                    IrmoMethodData *method_data)
{
        IrmoSListIterator iter;
        IrmoCallback *callback;
        IrmoInvokeCallback func;

        irmo_slist_iterate(list, &iter);

        while (irmo_slist_iter_has_more(&iter)) {

                callback = irmo_slist_iter_next(&iter);

                func = (IrmoInvokeCallback) callback->func;
                func(method_data, callback->user_data);
        }
}

// Sanity check a collection of method data.

static int irmo_method_data_check(IrmoMethodData *data)
{
	unsigned int i;
	
	// check all the arguments for sanity
	
	for (i=0; i<data->method->narguments; ++i) {
		IrmoMethodArg *arg = data->method->arguments[i];
		IrmoValue *value = &data->args[i];

		switch (arg->type) {
		case IRMO_TYPE_INT8:
			irmo_return_val_if_fail(value->i <= 0xff, 0);
			break;
		case IRMO_TYPE_INT16:
			irmo_return_val_if_fail(value->i <= 0xffff, 0);
			break;
		case IRMO_TYPE_INT32:
			break;
		case IRMO_TYPE_STRING:
			irmo_return_val_if_fail(value->s != NULL, 0);
			break;
                default:
                        irmo_return_val_if_fail(0, 0);
		}
	}
	
        // Success!

        return 1;
}

void irmo_method_internal_call(IrmoWorld *world, IrmoMethodData *data)
{
	// send to source

	if (world->remote) {
                irmo_connection_method_call(world->source_connection, data);
	}
	
	// invoke callback functions
	
	invoke_method_callbacks(&world->method_callbacks[data->method->index],
                                data);
}

void irmo_world_method_call(IrmoWorld *world, char *method_name, ...)
{
	IrmoMethodData method_data;
	IrmoMethod *method;
	IrmoValue *args;
	va_list arglist;
	unsigned int i;

	irmo_return_if_fail(world != NULL);
	irmo_return_if_fail(method_name != NULL);
	
	method = irmo_interface_get_method(world->iface, method_name);

	if (method == NULL) {
                irmo_warning_message("irmo_world_method_call",
                                     "unknown method '%s'", method_name);
                return;
	}

	args = irmo_new0(IrmoValue, method->narguments);

	// read each of the arguments
	
	va_start(arglist, method_name);
	
	for (i=0; i<method->narguments; ++i) {
		switch (method->arguments[i]->type) {
		case IRMO_TYPE_INT8:
		case IRMO_TYPE_INT16:
		case IRMO_TYPE_INT32:
			args[i].i = va_arg(arglist, unsigned int);
			break;
		case IRMO_TYPE_STRING:
			args[i].s = va_arg(arglist, char *);
			break;
                default:
                        irmo_bug();
                        break;
		}
	}

	va_end(arglist);

	method_data.method = method;
	method_data.args = args;
	method_data.src = NULL;

        if (irmo_method_data_check(&method_data)) {
                irmo_method_internal_call(world, &method_data);
        }

	free(args);
}


void irmo_world_method_call2(IrmoWorld *world, 
                             IrmoMethod *method,
			     IrmoValue *arguments)
{
	IrmoMethodData method_data;

	irmo_return_if_fail(world != NULL);
        irmo_return_if_fail(method != NULL);
        irmo_return_if_fail(method->iface == world->iface);
	
	method_data.method = method;
	method_data.args = arguments;
	method_data.src = NULL;

        if (irmo_method_data_check(&method_data)) {
                irmo_method_internal_call(world, &method_data);
        }
}


IrmoClient *irmo_method_get_source(IrmoMethodData *data)
{
	irmo_return_val_if_fail(data != NULL, NULL);
	
	return data->src;
}

char *irmo_method_arg_string(IrmoMethodData *data, char *argname)
{
	IrmoMethodArg *arg;

	irmo_return_val_if_fail(data != NULL, NULL);
	irmo_return_val_if_fail(argname != NULL, NULL);
	
	arg = irmo_method_get_argument(data->method, argname);

	if (arg == NULL) {
		irmo_warning_message("irmo_method_arg_string",
                        "unknown method argument '%s' for '%s' method",
                        data->method->name, argname);

		return NULL;
	}

	if (arg->type != IRMO_TYPE_STRING) {
                irmo_warning_message("irmo_method_arg_string",
                        "'%s' argument for '%s' method is not a string type",
                        argname, data->method->name);
                return NULL;
	}

	return data->args[arg->index].s;		
}

unsigned int irmo_method_arg_int(IrmoMethodData *data, char *argname)
{
	IrmoMethodArg *arg;

	irmo_return_val_if_fail(data != NULL, 0);
	irmo_return_val_if_fail(argname != NULL, 0);
	
	arg = irmo_method_get_argument(data->method, argname);

	if (arg == NULL) {
                irmo_warning_message("irmo_method_arg_int",
                        "unknown method argument '%s' for '%s' method",
                        data->method->name, argname);

                return 0;
	}

	switch (arg->type) {
	case IRMO_TYPE_INT8:
	case IRMO_TYPE_INT16:
	case IRMO_TYPE_INT32:
		return data->args[arg->index].i;
	default:
		irmo_warning_message("irmo_method_arg_int",
                        "'%s' argument for '%s' method is not an integer type",
                        argname, data->method->name);

		return 0;
	}
}

