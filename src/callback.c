// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id: callback.c,v 1.23 2003-06-09 21:06:50 sdh300 Exp $
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
#include "if_spec.h"
#include "object.h"
#include "universe.h"

// add a callback function to a list

void irmo_callbacklist_add(GSList **list,
			   gpointer func,
			   gpointer user_data)
{
	IrmoCallbackFuncData *callback;
	
	callback = g_new0(IrmoCallbackFuncData, 1);
	callback->func = func;
	callback->user_data = user_data;

	*list = g_slist_append(*list, callback);
}

// search a list and remove an entry from a list of IrmoCallbackFuncData
// objects

static gint callbacklist_remove_search(IrmoCallbackFuncData *func_data,
				       IrmoCallbackFuncData *data)
{
	return func_data->func != data->func
	    || func_data->user_data != data->user_data;
}

gboolean irmo_callbacklist_remove(GSList **list,
				  gpointer func,
				  gpointer user_data)
{
	IrmoCallbackFuncData *func_data;
	GSList *result;
	IrmoCallbackFuncData search_data = {
		func: func,
		user_data: user_data,
	};

	// search for function in list
	
	result = g_slist_find_custom(*list, &search_data,
				     (GCompareFunc) callbacklist_remove_search);

	if (result) {

		func_data = g_slist_nth_data(result, 0);

		// remove from list

		*list = g_slist_remove(*list, func_data);

		free(func_data);

		return TRUE;
	}

	return FALSE;
}

static void callbacklist_free_foreach(IrmoCallbackFuncData *callback,
				      gpointer user_data)
{
	free(callback);
}

void irmo_callbacklist_free(GSList *list)
{
	g_slist_foreach(list, (GFunc) callbacklist_free_foreach, NULL);
	g_slist_free(list);
}

// create a new callbackdata object for watching an object or class

IrmoCallbackData *callbackdata_new(ClassSpec *objclass)
{
	IrmoCallbackData *data;

	data = g_new0(IrmoCallbackData, 1);

	data->objclass = objclass;

	data->new_callbacks = NULL;
	data->class_callbacks = NULL;
	data->destroy_callbacks = NULL;
	data->variable_callbacks = g_new0(GSList *, objclass->nvariables);

	return data;
}

void callbackdata_free(IrmoCallbackData *data)
{
	int i;
	
	// free all class callbacks

	irmo_callbacklist_free(data->class_callbacks);

	// free destroy callbacks

	irmo_callbacklist_free(data->destroy_callbacks);
	
	// free all variable callbacks

	for (i=0; i<data->objclass->nvariables; ++i) {
		irmo_callbacklist_free(data->variable_callbacks[i]);
	}

	free(data->variable_callbacks);

	free(data);
}

struct raise_data {
	IrmoObject *object;
	gchar *variable;
};

static void callbackdata_raise_foreach(IrmoCallbackFuncData *callback, 
				       struct raise_data *raise_data)
{
	IrmoVarCallback func = (IrmoVarCallback) callback->func;

	func(raise_data->object, raise_data->variable,
	     callback->user_data);
}

void callbackdata_raise(IrmoCallbackData *data,
			 IrmoObject *object, gint variable_index)
{
	struct raise_data raise_data = {
		object: object,
		variable: data->objclass->variables[variable_index]->name,
	};

	// call class callbacks
	
	g_slist_foreach(data->class_callbacks,
			(GFunc) callbackdata_raise_foreach,
			&raise_data);

	// variable callbacks

	g_slist_foreach(data->variable_callbacks[variable_index],
			(GFunc) callbackdata_raise_foreach,
			&raise_data);
}

static void callbackdata_raise_destroy_foreach(IrmoCallbackFuncData *callback,
						struct raise_data *raise_data)
{
	IrmoObjCallback func = (IrmoObjCallback) callback->func;
	
	func(raise_data->object, callback->user_data);
}

void callbackdata_raise_destroy(IrmoCallbackData *data,
				 IrmoObject *object)
{
	struct raise_data raise_data = {
		object: object,
	};

	g_slist_foreach(data->destroy_callbacks,
			(GFunc) callbackdata_raise_destroy_foreach,
			&raise_data);
}

void callbackdata_raise_new(IrmoCallbackData *data, IrmoObject *object)
{
	struct raise_data raise_data = {
		object: object,
	};

	g_slist_foreach(data->new_callbacks,
			(GFunc) callbackdata_raise_destroy_foreach,
			&raise_data);
}

static GSList **find_variable(IrmoCallbackData *data, gchar *variable)
{
	if (variable) {
		ClassVarSpec *varspec
			= g_hash_table_lookup(data->objclass->variable_hash,
					      variable);
		if (varspec)
			return &data->variable_callbacks[varspec->index];
		else 
			return NULL;
	} else {
		return &data->class_callbacks;
	}
}

static gboolean callbackdata_watch(IrmoCallbackData *data,
				   gchar *variable,
				   IrmoVarCallback func, gpointer user_data)
{
	GSList **list;

	list = find_variable(data, variable);

	if (!list)
		return FALSE;

	irmo_callbacklist_add(list, func, user_data);

	return TRUE;
}

static gboolean callbackdata_unwatch(IrmoCallbackData *data,
				     gchar *variable,
				     IrmoVarCallback func, gpointer user_data)
{
	GSList **list;

	list = find_variable(data, variable);

	if (!list)
		return FALSE;

	return irmo_callbacklist_remove(list, func, user_data);
}

// watch creation of new objects of a particular class

void irmo_universe_watch_new(IrmoUniverse *universe, gchar *classname,
			     IrmoObjCallback func, gpointer user_data)
{
	IrmoCallbackData *data;
	ClassSpec *spec;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(classname != NULL);
	g_return_if_fail(func != NULL);
	
	// find the class

	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_watch_new: unknown class '%s'\n", 
			classname);
	} else {
		data = universe->callbacks[spec->index];
		
		irmo_callbacklist_add(&data->new_callbacks,
				      func, user_data);
	}
}

// stop watching new objects

void irmo_universe_unwatch_new(IrmoUniverse *universe, gchar *classname,
			       IrmoObjCallback func, gpointer user_data)
{
	IrmoCallbackData *data;
	ClassSpec *spec;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(classname != NULL);
	g_return_if_fail(func != NULL);
	
	// find the class

	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_unwatch_new: unknown class '%s'\n", 
			classname);
	} else {
		data = universe->callbacks[spec->index];
		
		// remove from new callbacks list for class
		
		if (!irmo_callbacklist_remove(&data->new_callbacks,
					      func, user_data)) {
			fprintf(stderr,
				"irmo_universe_unwatch_new: watch not "
				"found for new objects of class '%s'\n",
				classname);
		}
	}
}

void irmo_universe_watch_class(IrmoUniverse *universe,
			       gchar *classname, gchar *variable,
			       IrmoVarCallback func, gpointer user_data)
{
	ClassSpec *spec;
	
	g_return_if_fail(universe != NULL);
	g_return_if_fail(classname != NULL);
	g_return_if_fail(func != NULL);
	
	// find the class
	
	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_watch_class: unknown class '%s'\n",
			classname);
	} else {
		if (!callbackdata_watch(universe->callbacks[spec->index],
					variable,
					func, user_data)) {
			fprintf(stderr,
				"irmo_universe_watch_class: unknown "
				"variable '%s' in class '%s'\n",
				variable, classname);
		}
	}
}

void irmo_universe_unwatch_class(IrmoUniverse *universe,
				 gchar *classname, gchar *variable,
				 IrmoVarCallback func, gpointer user_data)
{
	ClassSpec *spec;
	
	g_return_if_fail(universe != NULL);
	g_return_if_fail(classname != NULL);
	g_return_if_fail(func != NULL);
	
	// find the class
	
	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_unwatch_class: unknown class '%s'\n",
			classname);
	} else {
		if (!callbackdata_unwatch(universe->callbacks[spec->index],
					  variable,
					  func, user_data)) {
			fprintf(stderr,
				"irmo_universe_unwatch_class: watch not "
				"found for class '%s' ",
				classname);
			if (variable)
				fprintf(stderr, "(variable '%s')\n", variable);
			else
				fprintf(stderr, "\n");
		}
	}
}

void irmo_universe_watch_destroy(IrmoUniverse *universe, gchar *classname,
				 IrmoObjCallback func, gpointer user_data)
{
	IrmoCallbackData *data;
	ClassSpec *spec;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(classname != NULL);
	g_return_if_fail(func != NULL);
	
	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_watch_destroy: unknown class '%s'\n",
			classname);
	} else {
		data = universe->callbacks[spec->index];
		
		irmo_callbacklist_add(&data->destroy_callbacks,
				      func, user_data);
	}
}

void irmo_universe_unwatch_destroy(IrmoUniverse *universe, gchar *classname,
				   IrmoObjCallback func, gpointer user_data)
{
	IrmoCallbackData *data;
	ClassSpec *spec;

	g_return_if_fail(universe != NULL);
	g_return_if_fail(classname != NULL);
	g_return_if_fail(func != NULL);
	
	spec = g_hash_table_lookup(universe->spec->class_hash, classname);

	if (!spec) {
		fprintf(stderr,
			"irmo_universe_unwatch_destroy: unknown class '%s'\n",
			classname);
	} else {
		data = universe->callbacks[spec->index];
		
		if (!irmo_callbacklist_remove(&data->destroy_callbacks,
					      func, user_data)) {
			fprintf(stderr,
				"irmo_universe_unwatch_destroy: watch not "
				"found for destroy in class '%s'\n", 
				classname);
		}
	}
}

void irmo_object_watch(IrmoObject *object, gchar *variable,
		       IrmoVarCallback func, gpointer user_data)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(func != NULL);
	
	if (!callbackdata_watch(object->callbacks, variable,
				func, user_data)) {
		fprintf(stderr,
			"irmo_object_watch: unknown variable '%s' "
			"in class '%s'\n",
			variable, object->objclass->name);
	}
}

void irmo_object_unwatch(IrmoObject *object, gchar *variable,
			 IrmoVarCallback func, gpointer user_data)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(func != NULL);
	
	if (!callbackdata_unwatch(object->callbacks, variable,
				  func, user_data)) {

		if (variable)
			fprintf(stderr,
				"irmo_object_unwatch: unknown variable "
				"or watch not found for object "
				"(class '%s', variable '%s')\n",
				object->objclass->name, variable);
		else
			fprintf(stderr,
				"irmo_object_unwatch: watch not found "
				"for object (class '%s')\n",
				object->objclass->name);
	}
}

void irmo_object_watch_destroy(IrmoObject *object,
			       IrmoObjCallback func, gpointer user_data)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(func != NULL);
	
	irmo_callbacklist_add(&object->callbacks->destroy_callbacks,
			      func, user_data);
}

void irmo_object_unwatch_destroy(IrmoObject *object,
				 IrmoObjCallback func, gpointer user_data)
{
	g_return_if_fail(object != NULL);
	g_return_if_fail(func != NULL);
	
	if (!irmo_callbacklist_remove(&object->callbacks->destroy_callbacks,
				      (IrmoVarCallback) func, user_data)) {
		fprintf(stderr,
			"irmo_object_unwatch_destroy: watch not found "
			"for destroy in class '%s'\n",
			object->objclass->name);
	}
}

// $Log: not supported by cvs2svn $
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
