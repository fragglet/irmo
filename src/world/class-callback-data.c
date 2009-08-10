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
#include "base/error.h"

#include "interface/interface.h"

#include "class-callback-data.h"
#include "object.h"
#include "world.h"

void irmo_class_callback_init(ClassCallbackData *data,
                              ClassCallbackData *parent_data,
                              IrmoClass *klass)
{
        data->new_callbacks = NULL;
        data->parent_data = parent_data;
        data->klass = klass;

        irmo_object_callback_init(&data->object_callbacks, klass);
}

void irmo_class_callback_free(ClassCallbackData *data)
{
        irmo_callback_list_free(&data->new_callbacks);

        irmo_object_callback_free(&data->object_callbacks, data->klass);
}

void irmo_class_callback_raise_new(ClassCallbackData *data,
                                   IrmoObject *object)
{
        ClassCallbackData *data_iter;

        // Iterate up through all parent classes, invoking callbacks
        // for each class:

        data_iter = data;

        while (data_iter != NULL) {

                irmo_obj_callbacks_invoke(&data_iter->new_callbacks, object);

                data_iter = data_iter->parent_data;
        }
}

void irmo_class_callback_raise(ClassCallbackData *data,
                               IrmoObject *object,
                               unsigned int variable_index)
{
        ClassCallbackData *data_iter;

        // Iterate up through all parent classes, invoking callbacks
        // for each class.  When we reach the "top level" class
        // structure (callbacks_all, data_iter->klass == NULL),
        // there are no more variable callbacks to invoke, because
        // the top level structure has no variables.

        data_iter = data;

        while (data_iter != NULL && data_iter->klass != NULL) {

                // If this is outside the range of variables for this
                // class, it is a change to a variable from a subclass.
                // Stop.

                if (variable_index >= data_iter->klass->nvariables) {
                        break;
                }

                irmo_object_callback_raise(&data_iter->object_callbacks,
                                           object,
                                           variable_index);

                data_iter = data_iter->parent_data;
        }
}

void irmo_class_callback_raise_destroy(ClassCallbackData *data, 
                                       IrmoObject *object)
{
        ClassCallbackData *data_iter;

        // Iterate up through all parent classes, invoking callbacks
        // for each class:

        data_iter = data;

        while (data_iter != NULL) {

                irmo_object_callback_raise_destroy(&data_iter->object_callbacks,
                                                   object);

                data_iter = data_iter->parent_data;
        }
}

IrmoCallback *irmo_class_callback_watch(ClassCallbackData *data,
                                        char *variable_name,
                                        IrmoVarCallback func,
                                        void *user_data)
{
        return irmo_object_callback_watch(&data->object_callbacks,
                                          data->klass,
                                          variable_name,
                                          func,
                                          user_data);
}

IrmoCallback *irmo_class_callback_watch_destroy(ClassCallbackData *data,
                                                IrmoObjCallback func,
                                                void *user_data)
{
        return irmo_object_callback_watch_destroy(&data->object_callbacks,
                                                  func,
                                                  user_data);
}

IrmoCallback *irmo_class_callback_watch_new(ClassCallbackData *data,
                                            IrmoObjCallback func,
                                            void *user_data)
{
        return irmo_callback_list_add(&data->new_callbacks, func, user_data);
}

