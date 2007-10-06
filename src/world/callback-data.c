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

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "interface/interface.h"

#include "callback-data.h"
#include "object.h"
#include "world.h"

// create a new callback_data object for watching an object or class

IrmoCallbackData *irmo_callback_data_new(IrmoClass *objclass, 
                                         IrmoCallbackData *parent)
{
	IrmoCallbackData *data;

	data = irmo_new0(IrmoCallbackData, 1);

	data->objclass = objclass;

	data->new_callbacks = NULL;
	data->class_callbacks = NULL;
	data->destroy_callbacks = NULL;

	data->parent_data = parent;

	if (objclass) {
		data->variable_callbacks 
			= irmo_new0(IrmoCallbackList, objclass->nvariables);
        }

	return data;
}

void irmo_callback_data_free(IrmoCallbackData *data)
{
	unsigned int i;
	
	// free all class callbacks

	irmo_callback_list_free(&data->class_callbacks);

	// free new object callbacks
	
	irmo_callback_list_free(&data->new_callbacks);

	// free destroy callbacks

	irmo_callback_list_free(&data->destroy_callbacks);
	
	if (data->objclass) {

		// free all variable callbacks

		for (i=0; i<data->objclass->nvariables; ++i) {
			irmo_callback_list_free(&data->variable_callbacks[i]);
		}

		free(data->variable_callbacks);
	}

	free(data);
}

static void callback_data_invoke_callbacks(IrmoCallbackList *list,
                                           IrmoObject *obj,
                                           char *variable)
{
        IrmoSListIterator *iter;
        IrmoCallback *callback;
	IrmoVarCallback func;

        iter = irmo_slist_iterate(list);

        while (irmo_slist_iter_has_more(iter)) {
                callback = irmo_slist_iter_next(iter);

                func = (IrmoVarCallback) callback->func;
                func(obj, variable, callback->user_data);
        }

        irmo_slist_iter_free(iter);
}

void irmo_callback_data_raise(IrmoCallbackData *data,
			     IrmoObject *object, 
                             unsigned int variable_index)
{
        char *variable_name = 
		object->objclass->variables[variable_index]->name;

	// call class callbacks
       	
        callback_data_invoke_callbacks(&data->class_callbacks,
                                       object, variable_name);

	if (data->objclass) {
		// variable callbacks

                callback_data_invoke_callbacks(
                            &data->variable_callbacks[variable_index],
                            object, variable_name);
	}

	// recurse through superclass watches
	// but not if this is a variable that isnt in the superclass

	if (data->parent_data
	 && variable_index < data->parent_data->objclass->nvariables) {
		irmo_callback_data_raise(data->parent_data, object, 
					variable_index);
	}
}

// Used by raise_new and raise_destroy, below.
// Go through a list of IrmoObjCallback callback functions and invoke 
// them all.

static void callback_data_invoke_obj_callbacks(IrmoCallbackList *list,
                                               IrmoObject *object)
{
        IrmoSListIterator *iter;
        IrmoCallback *callback;
	IrmoObjCallback func;

        iter = irmo_slist_iterate(list);

        while (irmo_slist_iter_has_more(iter)) {

                callback = irmo_slist_iter_next(iter);

                func = (IrmoObjCallback) callback->func;
                func(object, callback->user_data);
        }

        irmo_slist_iter_free(iter);
}

void irmo_callback_data_raise_destroy(IrmoCallbackData *data,
				     IrmoObject *object)
{
        callback_data_invoke_obj_callbacks(&data->destroy_callbacks,
                                           object);

	// recurse through superclass watches

	if (data->parent_data) {
		irmo_callback_data_raise_destroy(data->parent_data, object);
        }
}

void irmo_callback_data_raise_new(IrmoCallbackData *data, IrmoObject *object)
{
        callback_data_invoke_obj_callbacks(&data->new_callbacks, object);

	// recurse through superclass watches

	if (data->parent_data) {
		irmo_callback_data_raise_new(data->parent_data, object);
        }
}

static IrmoCallbackList *find_variable(IrmoCallbackData *data, char *variable)
{
	if (variable) {
		IrmoClassVar *classvar;

		// cannot specify a variable name and no classname

		if (data->objclass) {
			return NULL;
                }

		classvar = irmo_class_get_variable(data->objclass, variable);

		if (classvar) {
			return &data->variable_callbacks[classvar->index];
		} else {
			return NULL;
                }
	} else {
		return &data->class_callbacks;
	}
}

static IrmoCallback *callback_data_watch(IrmoCallbackData *data,
                                         char *variable,
                                         IrmoVarCallback func, 
                                         void *user_data)
{
	IrmoCallbackList *list;

	list = find_variable(data, variable);

	if (!list) {
		return 0;
        }

	return irmo_callback_list_add(list, func, user_data);
}

static IrmoCallbackData *find_callback_class(IrmoWorld *world, char *classname)
{
	IrmoClass *klass;

	if (classname == NULL) 
		return world->callbacks_all;

	klass = irmo_interface_get_class(world->iface, classname);

	if (!klass)
		return NULL;

	return world->callbacks[klass->index];
}

// watch creation of new objects of a particular class

IrmoCallback *irmo_world_watch_new(IrmoWorld *world, char *classname,
                                   IrmoObjCallback func, void *user_data)
{
	IrmoCallbackData *data;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	// find the class

	data = find_callback_class(world, classname);

	if (!data) {
		irmo_error_report("irmo_world_watch_new",
                                  "unknown class '%s'", classname);
		return NULL;
	} else {
		return irmo_callback_list_add(&data->new_callbacks,
                                              func, user_data);
	}
}

IrmoCallback *irmo_world_watch_class(IrmoWorld *world,
				     char *classname, char *variable,
				     IrmoVarCallback func, 
				     void *user_data)
{
	IrmoCallbackData *data;
	IrmoCallback *callback = NULL;
	
	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	irmo_return_val_if_fail(!(classname == NULL && variable != NULL), NULL);
	
	// find the class
	
	data = find_callback_class(world, classname);

	if (!data) {
		irmo_error_report("irmo_world_watch_class",
				  "unknown class '%s'", classname);
	} else {
	        callback = callback_data_watch(data,
                                               variable,
                                               func, user_data);

		if (!callback) {
			irmo_error_report("irmo_world_watch_class",
					  "unknown variable '%s' in class '%s'",
					  variable, classname);
		}
	}

	return callback;
}

IrmoCallback *irmo_world_watch_destroy(IrmoWorld *world, 
				       char *classname,
				       IrmoObjCallback func, 
				       void *user_data)
{
	IrmoCallbackData *data;
	IrmoCallback *callback = NULL;

	irmo_return_val_if_fail(world != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	data = find_callback_class(world, classname);

	if (!data) {
		irmo_error_report("irmo_world_watch_destroy",
                                  "unknown class '%s'", classname);
	} else {
		callback = irmo_callback_list_add(&data->destroy_callbacks,
                                                  func, user_data);
	}

	return callback;
}

IrmoCallback *irmo_object_watch(IrmoObject *object, char *variable,
				IrmoVarCallback func, void *user_data)
{
        IrmoCallback *callback;

	irmo_return_val_if_fail(object != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);

	callback = callback_data_watch(object->callbacks, variable,
                                       func, user_data);

	if (!callback) {
		irmo_error_report("irmo_object_watch",
				  "unknown variable '%s' in class '%s'",
				  variable, object->objclass->name);
	}

	return callback;
}

IrmoCallback *irmo_object_watch_destroy(IrmoObject *object,
					IrmoObjCallback func, 
					void *user_data)
{
	irmo_return_val_if_fail(object != NULL, NULL);
	irmo_return_val_if_fail(func != NULL, NULL);
	
	return irmo_callback_list_add(&object->callbacks->destroy_callbacks,
                                      func, user_data);
}

