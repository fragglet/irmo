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

#include "sysheaders.h"

#include "callback.h"
#include "error.h"
#include "if_spec.h"
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

static void irmo_callback_destroy_foreach(IrmoCallback *callback,
					  IrmoCallback *parent)
{
	IrmoCallbackCallback func = callback->func;

	func(parent, callback->user_data);
}

static void irmo_callback_destroy(IrmoCallback *callback)
{
	// invoke all the callbacks watching for this callback
	// being destroyed

	irmo_slist_foreach(callback->destroy_callbacks, 
			(IrmoSListIterator) irmo_callback_destroy_foreach, 
			callback);

	// free callback data

	irmo_callbacklist_free(callback->destroy_callbacks);
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

static void callbacklist_free_foreach(IrmoCallback *callback,
				      void *user_data)
{
	irmo_callback_destroy(callback);
}

void irmo_callbacklist_free(IrmoSListEntry *list)
{
	irmo_slist_foreach(list, (IrmoSListIterator) callbacklist_free_foreach, NULL);
	irmo_slist_free(list);
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

	irmo_callbacklist_free(data->class_callbacks);

	// free new object callbacks
	
	irmo_callbacklist_free(data->new_callbacks);

	// free destroy callbacks

	irmo_callbacklist_free(data->destroy_callbacks);
	
	if (data->objclass) {

		// free all variable callbacks

		for (i=0; i<data->objclass->nvariables; ++i) {
			irmo_callbacklist_free(data->variable_callbacks[i]);
		}

		free(data->variable_callbacks);
	}

	free(data);
}

struct raise_data {
	IrmoObject *object;
	char *variable;
};

static void callbackdata_raise_foreach(IrmoCallback *callback, 
				       struct raise_data *raise_data)
{
	IrmoVarCallback func = (IrmoVarCallback) callback->func;

	func(raise_data->object, raise_data->variable,
	     callback->user_data);
}

void irmo_callbackdata_raise(IrmoCallbackData *data,
			     IrmoObject *object, int variable_index)
{
	struct raise_data raise_data = {
		object,
		object->objclass->variables[variable_index]->name,
	};

	// call class callbacks
	
	irmo_slist_foreach(data->class_callbacks,
			(IrmoSListIterator) callbackdata_raise_foreach,
			&raise_data);

	if (data->objclass) {
		// variable callbacks

		irmo_slist_foreach(data->variable_callbacks[variable_index],
				(IrmoSListIterator) callbackdata_raise_foreach,
				&raise_data);
	}

	// recurse through superclass watches
	// but not if this is a variable that isnt in the superclass

	if (data->parent_data
	 && variable_index < data->parent_data->objclass->nvariables) {
		irmo_callbackdata_raise(data->parent_data, object, 
					variable_index);
	}
}

static void callbackdata_raise_destroy_foreach(IrmoCallback *callback,
						struct raise_data *raise_data)
{
	IrmoObjCallback func = (IrmoObjCallback) callback->func;
	
	func(raise_data->object, callback->user_data);
}

void irmo_callbackdata_raise_destroy(IrmoCallbackData *data,
				     IrmoObject *object)
{
	struct raise_data raise_data = {
		object,
	};

	irmo_slist_foreach(data->destroy_callbacks,
			(IrmoSListIterator) callbackdata_raise_destroy_foreach,
			&raise_data);

	// recurse through superclass watches

	if (data->parent_data)
		irmo_callbackdata_raise_destroy(data->parent_data, object);
}

void irmo_callbackdata_raise_new(IrmoCallbackData *data, IrmoObject *object)
{
	struct raise_data raise_data = {
		object,
	};

	irmo_slist_foreach(data->new_callbacks,
			(IrmoSListIterator) callbackdata_raise_destroy_foreach,
			&raise_data);

	// recurse through superclass watches

	if (data->parent_data)
		irmo_callbackdata_raise_new(data->parent_data, object);
}

static IrmoSListEntry **find_variable(IrmoCallbackData *data, char *variable)
{
	if (variable) {
		IrmoClassVar *varspec;

		// cannot specify a variable name and no classname

		if (data->objclass)
			return NULL;

		varspec = irmo_class_get_variable(data->objclass, variable);

		if (varspec)
			return &data->variable_callbacks[varspec->index];
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
	IrmoClass *spec;

	if (classname == NULL) 
		return world->callbacks_all;

	spec = irmo_interface_spec_get_class(world->spec, classname);

	if (!spec)
		return NULL;

	return world->callbacks[spec->index];
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

// $Log$
// Revision 1.19  2005/12/25 00:48:29  fraggle
// Use internal memory functions, rather than the glib ones
//
// Revision 1.18  2005/12/25 00:38:18  fraggle
// Use internal macros instead of glib ones for assertation checks
//
// Revision 1.17  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.16  2004/06/23 14:33:02  fraggle
// Use prepend instead of append for linked lists
//
// Revision 1.15  2004/04/17 22:19:57  fraggle
// Use glib memory management functions where possible
//
// Revision 1.14  2003/12/27 19:22:25  fraggle
// Some of the callback lists were not being destroyed properly
//
// Revision 1.13  2003/12/27 19:01:48  fraggle
// irmo_callback_watch_destroy
//
// Revision 1.12  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.11  2003/11/18 18:14:45  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.10  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.9  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.8  2003/09/02 20:33:55  fraggle
// Subclassing in interfaces
//
// Revision 1.7  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.6  2003/08/28 16:43:45  fraggle
// Use the reflection API internally to improve readability in places
//
// Revision 1.5  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.4  2003/08/16 16:45:11  fraggle
// Allow watches on all objects regardless of class
//
// Revision 1.3  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
// Revision 1.2  2003/07/22 02:05:39  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:23  fraggle
// Initial sourceforge import
//
// Revision 1.23  2003/06/09 21:06:50  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.22  2003/04/25 01:26:18  sdh300
// Add glib assertations to all public API functions
//
// Revision 1.21  2003/03/16 00:44:04  sdh300
// Add irmo_callbacklist_free function
//
// Revision 1.20  2003/03/14 18:31:36  sdh300
// Generalise callback functions to irmo_callbacklist type,
// remove redundant client_callback code
//
// Revision 1.19  2003/03/07 14:31:18  sdh300
// Callback functions for watching new client connects
//
// Revision 1.18  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.17  2003/02/23 01:01:00  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.16  2003/02/23 00:05:02  sdh300
// Fix compile after header changes
//
// Revision 1.15  2002/11/21 13:08:09  sdh300
// printf should be to stderr
//
// Revision 1.14  2002/11/12 23:25:47  sdh300
// more clarity (missed this in previous commit)
//
// Revision 1.13  2002/11/12 23:13:04  sdh300
// add some clarity
//
// Revision 1.12  2002/11/12 23:04:30  sdh300
// callback removal, and code cleanup/generalisation
//
// Revision 1.11  2002/11/05 22:33:26  sdh300
// more name changes
//
// Revision 1.10  2002/11/05 22:20:44  sdh300
// oops
//
// Revision 1.9  2002/11/05 16:28:10  sdh300
// new object callbacks
//
// Revision 1.8  2002/11/05 16:00:36  sdh300
// various "oops"'es
//
// Revision 1.7  2002/11/05 15:55:12  sdh300
// object destroy callbacks
//
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
