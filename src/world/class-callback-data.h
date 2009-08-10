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

#ifndef IRMO_WORLD_CLASS_CALLBACK_DATA_H
#define IRMO_WORLD_CLASS_CALLBACK_DATA_H

/*!
 * Class callback data.
 *
 * This structure encapsulates the different callback lists used to
 * watch events related to all objects of a particular class.  This
 * is an extension of the @ref ObjectCallbackData structure (it also
 * monitors creation of new objects).
 *
 * Each @ref IrmoWorld has a class callback structure for each class
 * in the interface used by the world, plus a "top-level" class
 * callback structure for monitoring events related to objects of
 * all classes.
 */

typedef struct _ClassCallbackData ClassCallbackData;

#include "interface/class.h"
#include "object-callback-data.h"

struct _ClassCallbackData {

        // Class that this applies to.

        IrmoClass *klass;

        // Parent class callback data structure.
        // For the top-level callbacks structure, this is NULL.

        ClassCallbackData *parent_data;

        // List of callbacks for watching creation of new objects.

        IrmoCallbackList new_callbacks;

        // Other callback lists in common with those used for
        // object callbacks.

        ObjectCallbackData object_callbacks;
};

/*!
 * Initialise a @ref ClassCallbackData structure.
 *
 * @param data           The structure to initialise.
 * @param parent_data    Structure for the parent class of this class.
 * @param klass          The class for this structure.
 */

void irmo_class_callback_init(ClassCallbackData *data,
                              ClassCallbackData *parent_data,
                              IrmoClass *klass);

/*! 
 * Free data used for the specified @ref ClassCallbackData.
 *
 * @param data           The structure.
 */

void irmo_class_callback_free(ClassCallbackData *data);

/*!
 * Invoke callback functions in response to a variable of an object belonging
 * to a particular class being changed.
 *
 * @param data           The @ref ClassCallbackData for the class.
 * @param object         The object that was changed.
 * @param variable_index The index of the variable that was changed.
 */

void irmo_class_callback_raise(ClassCallbackData *data,
                               IrmoObject *object,
                               unsigned int variable_index);

/*!
 * Invoke callback functions in response to an object of a particular class
 * being destroyed.
 *
 * @param data           The @ref ClassCallbackData for the class.
 * @param object         The object being destroyed.
 */

void irmo_class_callback_raise_destroy(ClassCallbackData *data, 
                                       IrmoObject *object);
/*!
 * Invoke callback functions in response to an object of a particular class
 * being created.
 *
 * @param data           The @ref ClassCallbackData for the class.
 * @param object         The new object.
 */

void irmo_class_callback_raise_new(ClassCallbackData *data, 
                                   IrmoObject *object);

/*!
 * Watch for changes to variables of objects of a particular class.
 *
 * @param data           The @ref ClassCallbackData for the class.
 * @param variable_name  Name of the variable to watch, or NULL
 *                       to watch for changes to all variables.
 * @param func           The callback function to invoke.
 * @param user_data      Extra data to pass to the callback function.
 * @return               @ref IrmoCallback object to return representing
 *                       the callback.
 */

IrmoCallback *irmo_class_callback_watch(ClassCallbackData *data,
                                        char *variable_name,
                                        IrmoVarCallback func,
                                        void *user_data);
/*!
 * Watch for when objects of a particular class are instantiated.
 *
 * @param data           The @ref ClassCallbackData for the class.
 * @param func           The callback function to invoke.
 * @param user_data      Extra data to pass to the callback function.
 * @return               @ref IrmoCallback object to return representing
 *                       the callback.
 */


IrmoCallback *irmo_class_callback_watch_new(ClassCallbackData *data,
                                            IrmoObjCallback func,
                                            void *user_data);

/*!
 * Watch for when objects of a particular class are destroyed.
 *
 * @param data           The @ref ClassCallbackData for the class.
 * @param func           The callback function to invoke.
 * @param user_data      Extra data to pass to the callback function.
 * @return               @ref IrmoCallback object to return representing
 *                       the callback.
 */

IrmoCallback *irmo_class_callback_watch_destroy(ClassCallbackData *data,
                                                IrmoObjCallback func,
                                                void *user_data);

#endif /* #ifndef IRMO_WORLD_CLASS_CALLBACK_DATA_H */

