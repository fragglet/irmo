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
#include "algo/slist.h"
#include "base/util.h"
#include "base/error.h"

#include "callback.h"

// add a callback function to a list

IrmoCallback *irmo_callback_list_add(IrmoCallbackList *list,
                                     void *func, void *user_data)
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
                                          IrmoCallbackList *list)
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

	irmo_callback_list_free(&callback->destroy_callbacks);
	free(callback);
}

// unset a callback

void irmo_callback_unset(IrmoCallback *callback)
{
        IrmoCallbackList *list = callback->list;

	irmo_return_if_fail(callback != NULL);

	irmo_slist_remove_data(list, irmo_pointer_equal, callback);

	irmo_callback_destroy(callback);
}

void irmo_callback_list_free(IrmoCallbackList *list)
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
	return irmo_callback_list_add(&callback->destroy_callbacks,
				      func,
				      user_data);
}

