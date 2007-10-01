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

#include "callback.h"
#include "object.h"
#include "world.h"

// add a callback function to a list

IrmoCallback *irmo_callbacklist_add(IrmoSListEntry **list,
				    void *func,
				    void *user_data)
{
	IrmoCallback *callback;
	
	callback = irmo_new0(IrmoCallback, 1);
	callback->func = func;
	callback->user_data = user_data;
	callback->list = list;

        irmo_slist_prepend(list, callback);

	return callback;
}

static void irmo_invoke_destroy_callbacks(IrmoCallback *parent,
                                          IrmoSListEntry **list)
{
        IrmoSListIterator *iter;
        IrmoCallback *callback;
	IrmoCallbackCallback func;

        iter = irmo_slist_iterate(list);

        while (irmo_slist_iter_has_more(iter)) {
                callback = irmo_slist_iter_next(iter);

                func = callback->func;
                func(parent, callback->user_data);
        }

        irmo_slist_iter_free(iter);
}

static void irmo_callback_destroy(IrmoCallback *callback)
{
	// invoke all the callbacks watching for this callback
	// being destroyed

        irmo_invoke_destroy_callbacks(callback, &callback->destroy_callbacks);

	// free callback data

	irmo_callbacklist_free(&callback->destroy_callbacks);
	free(callback);
}

// unset a callback

void irmo_callback_unset(IrmoCallback *callback)
{
        IrmoSListEntry **list = callback->list;

	irmo_return_if_fail(callback != NULL);

	irmo_slist_remove_data(list, irmo_pointer_equal, callback);

	irmo_callback_destroy(callback);
}

void irmo_callbacklist_free(IrmoSListEntry **list)
{
        IrmoSListIterator *iter;
        IrmoCallback *callback;

        iter = irmo_slist_iterate(list);

        while (irmo_slist_iter_has_more(iter)) {
                callback = irmo_slist_iter_next(iter);

                irmo_callback_destroy(callback);
        }

        irmo_slist_iter_free(iter);

	irmo_slist_free(*list);
}

// watch for when a callback is destroyed

IrmoCallback *irmo_callback_watch_destroy(IrmoCallback *callback,
					  IrmoCallbackCallback func,
					  void *user_data)
{
	return irmo_callbacklist_add(&callback->destroy_callbacks,
				     func,
				     user_data);
}

// create a new callbackdata object for watching an object or class

IrmoCallbackData *irmo_callbackdata_new(IrmoClass *objclass, 
					IrmoCallbackData *parent)
{
	IrmoCallbackData *data;

	data = irmo_new0(IrmoCallbackData, 1);

	data->objclass = objclass;

	data->new_callbacks = NULL;
	data->class_callbacks = NULL;
	data->destroy_callbacks = NULL;

	data->parent_data = parent;

	if (objclass)
		data->variable_callbacks 
			= irmo_new0(IrmoSListEntry *, objclass->nvariables);

	return data;
}

void irmo_callbackdata_free(IrmoCallbackData *data)
{
	int i;
	
	// free all class callbacks

	irmo_callbacklist_free(&data->class_callbacks);

	// free new object callbacks
	
	irmo_callbacklist_free(&data->new_callbacks);

	// free destroy callbacks

	irmo_callbacklist_free(&data->destroy_callbacks);
	
	if (data->objclass) {

		// free all variable callbacks

		for (i=0; i<data->objclass->nvariables; ++i) {
			irmo_callbacklist_free(&data->variable_callbacks[i]);
		}

		free(data->variable_callbacks);
	}

	free(data);
}

static void irmo_callbackdata_invoke_callbacks(IrmoSListEntry **list,
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

void irmo_callbackdata_raise(IrmoCallbackData *data,
			     IrmoObject *object, int variable_index)
{
        char *variable_name = 
		object->objclass->variables[variable_index]->name;

	// call class callbacks
       	
        irmo_callbackdata_invoke_callbacks(&data->class_callbacks,
                                           object, variable_name);

	if (data->objclass) {
		// variable callbacks

                irmo_callbackdata_invoke_callbacks(
                                &data->variable_callbacks[variable_index],
                                object, variable_name);
	}

	// recurse through superclass watches
	// but not if this is a variable that isnt in the superclass

	if (data->parent_data
	 && variable_index < data->parent_data->objclass->nvariables) {
		irmo_callbackdata_raise(data->parent_data, object, 
					variable_index);
	}
}

// Used by raise_new and raise_destroy, below.
// Go through a list of IrmoObjCallback callback functions and invoke 
// them all.

static void irmo_callbackdata_invoke_obj_callbacks(IrmoSListEntry **list,
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

void irmo_callbackdata_raise_destroy(IrmoCallbackData *data,
				     IrmoObject *object)
{
        irmo_callbackdata_invoke_obj_callbacks(&data->destroy_callbacks,
                                               object);

	// recurse through superclass watches

	if (data->parent_data) {
		irmo_callbackdata_raise_destroy(data->parent_data, object);
        }
}

void irmo_callbackdata_raise_new(IrmoCallbackData *data, IrmoObject *object)
{
        irmo_callbackdata_invoke_obj_callbacks(&data->new_callbacks, object);

	// recurse through superclass watches

	if (data->parent_data) {
		irmo_callbackdata_raise_new(data->parent_data, object);
        }
}

static IrmoSListEntry **find_variable(IrmoCallbackData *data, char *variable)
{
	if (variable) {
		IrmoClassVar *classvar;

		// cannot specify a variable name and no classname

		if (data->objclass)
			return NULL;

		classvar = irmo_class_get_variable(data->objclass, variable);

		if (classvar)
			return &data->variable_callbacks[classvar->index];
		else 
			return NULL;
	} else {
		return &data->class_callbacks;
	}
}

static IrmoCallback *callbackdata_watch(IrmoCallbackData *data,
					char *variable,
					IrmoVarCallback func, 
					void *user_data)
{
	IrmoSListEntry **list;

	list = find_variable(data, variable);

	if (!list)
		return 0;

	return irmo_callbacklist_add(list, func, user_data);
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
		return irmo_callbacklist_add(&data->new_callbacks,
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
	        callback = callbackdata_watch(data,
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
		callback = irmo_callbacklist_add(&data->destroy_callbacks,
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

	callback = callbackdata_watch(object->callbacks, variable,
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
	
	return irmo_callbacklist_add(&object->callbacks->destroy_callbacks,
				     func, user_data);
}

