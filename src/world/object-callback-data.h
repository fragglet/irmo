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

//
// Callbacks
//

#ifndef IRMO_WORLD_OBJECT_CALLBACK_DATA_H
#define IRMO_WORLD_OBJECT_CALLBACK_DATA_H

typedef struct _ObjectCallbackData ObjectCallbackData;

#include "base/callback.h"

struct _ObjectCallbackData {

	// Callbacks for if any variable is changed

	IrmoCallbackList all_variable_callbacks;

	// Callbacks for if a particular variable is changed.
	
	IrmoCallbackList *variable_callbacks;

	// Callbacks called when object is destroyed
	
	IrmoCallbackList destroy_callbacks;
};

/*!
 * Initialise a @ref ObjectCallbackData structure.
 *
 * @param callback_data   The structure to initialise.
 * @param klass           The class of object.
 */

void irmo_object_callback_init(ObjectCallbackData *callback_data,
                               IrmoClass *klass);

/*!
 * Free data in use by a @ref ObjectCallbackData structure.
 *
 * @param callback_data   The structure.
 * @param klass           The class of object.
 */

void irmo_object_callback_free(ObjectCallbackData *callback_data,
                               IrmoClass *klass);

/*!
 * Invoke callback functions in response to a variable of an object being
 * changed.
 *
 * @param data            The callback data structure.
 * @param object          The object that changed.
 * @param variable_index  Index of the variable that changed.
 */

void irmo_object_callback_raise(ObjectCallbackData *data,
                                IrmoObject *object,
                                unsigned int variable_index);

/*!
 * Invoke callback functions in response to an object being destroyed.
 *
 * @param data            The callback data structure.
 * @param object          The object being destroyed.
 */

void irmo_object_callback_raise_destroy(ObjectCallbackData *data, 
                                        IrmoObject *object);

/*!
 * Watch for changes to an object.
 *
 * @param data           The callback data structure.
 * @param klass          The class of object to watch.
 * @param variable_name  Name of the variable to watch, or NULL
 *                       to watch for changes to all variables.
 * @param func           The callback function to invoke.
 * @param user_data      Extra data to pass to the callback function.
 * @return               @ref IrmoCallback object to return representing
 *                       the callback.
 */

IrmoCallback *irmo_object_callback_watch(ObjectCallbackData *data,
                                         IrmoClass *klass,
                                         char *variable_name,
                                         IrmoVarCallback func,
                                         void *user_data);

/*!
 * Watch for an object being destroyed.
 *
 * @param data           The callback data structure.
 * @param func           The callback function to invoke.
 * @param user_data      Extra data to pass to the callback function.
 * @return               @ref IrmoCallback object to return representing
 *                       the callback.
 */

IrmoCallback *irmo_object_callback_watch_destroy(ObjectCallbackData *data,
                                                 IrmoObjCallback func,
                                                 void *user_data);

/*!
 * Invoke all callback functions in a list of callbacks.
 *
 * @param list           The list of callback functions.
 * @param object         The object to pass to the callback functions.
 */

void irmo_obj_callbacks_invoke(IrmoCallbackList *list,
                               IrmoObject *object);

#endif /* #ifndef IRMO_WORLD_OBJECT_CALLBACK_DATA_H */

