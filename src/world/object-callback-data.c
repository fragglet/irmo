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
#include "base/callback.h"

#include "interface/interface.h"

#include "object.h"
#include "world.h"

// Used by raise_new and raise_destroy.
// Go through a list of IrmoObjCallback callback functions and invoke 
// them all.

void irmo_obj_callbacks_invoke(IrmoCallbackList *list,
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

// Go through a list of IrmoVarCallback callback functions and invoke
// them all.

static void irmo_var_callbacks_invoke(IrmoCallbackList *list,
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

// Initialise an ObjectCallbackData structure.

void irmo_object_callback_init(ObjectCallbackData *data,
                               IrmoClass *klass)
{
        data->all_variable_callbacks = NULL;
        data->destroy_callbacks = NULL;

        if (klass != NULL) {
                data->variable_callbacks = irmo_new0(IrmoCallbackList,
                                                     klass->nvariables);
        } else {
                data->variable_callbacks = NULL;
        }
}

// Free the contents of an ObjectCallbackData structure.

void irmo_object_callback_free(ObjectCallbackData *data,
                               IrmoClass *klass)
{
        unsigned int i;

        irmo_callback_list_free(&data->all_variable_callbacks);

        irmo_callback_list_free(&data->destroy_callbacks);

        if (data->variable_callbacks != NULL) {
                for (i=0; i<klass->nvariables; ++i) {
                        irmo_callback_list_free(&data->variable_callbacks[i]);
                }

                free(data->variable_callbacks);
        }
}

// Raise a variable change callback.

void irmo_object_callback_raise(ObjectCallbackData *data,
                                IrmoObject *object,
                                unsigned int variable_index)
{
        IrmoCallbackList *callback_list;
        char *variable_name;
      
        // Top-level class callback data with no proper class associated?
        // This has no variable callbacks, so just return.

        if (data->variable_callbacks == NULL) {
                return;
        }

        callback_list = &data->variable_callbacks[variable_index];
        variable_name = object->objclass->variables[variable_index]->name;

        irmo_var_callbacks_invoke(callback_list,
                                  object,
                                  variable_name);
}

void irmo_object_callback_raise_destroy(ObjectCallbackData *data, 
                                        IrmoObject *object)
{
        irmo_obj_callbacks_invoke(&data->destroy_callbacks,
                                  object);
}

IrmoCallback *irmo_object_callback_watch(ObjectCallbackData *data,
                                         IrmoClass *klass,
                                         char *variable_name,
                                         IrmoVarCallback func,
                                         void *user_data)
{
        IrmoCallbackList *callback_list;
        IrmoClassVar *class_var;

        // Use the "all variables" callback list or the one specific 
        // to the variable we want to watch.

        if (variable_name == NULL) {
                callback_list = &data->all_variable_callbacks;
        } else {
                class_var = irmo_class_get_variable(klass, variable_name);

                if (class_var == NULL) {
                        return NULL;
                }

                callback_list = &data->variable_callbacks[class_var->index];
        }

        return irmo_callback_list_add(callback_list, func, user_data);
}

IrmoCallback *irmo_object_callback_watch_destroy(ObjectCallbackData *data,
                                                 IrmoObjCallback func,
                                                 void *user_data)
{
        return irmo_callback_list_add(&data->destroy_callbacks,
                                      func,
                                      user_data);
}
