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

void irmo_object_callback_init(ObjectCallbackData *callback_data,
                               IrmoClass *klass);
void irmo_object_callback_free(ObjectCallbackData *callback_data,
                               IrmoClass *klass);
void irmo_object_callback_raise(ObjectCallbackData *data,
                                IrmoObject *object,
                                unsigned int variable_index);
void irmo_object_callback_raise_destroy(ObjectCallbackData *data, 
                                        IrmoObject *object);
IrmoCallback *irmo_object_callback_watch(ObjectCallbackData *data,
                                         IrmoClass *klass,
                                         char *variable_name,
                                         IrmoVarCallback func,
                                         void *user_data);
IrmoCallback *irmo_object_callback_watch_destroy(ObjectCallbackData *data,
                                                 IrmoObjCallback func,
                                                 void *user_data);

void irmo_obj_callbacks_invoke(IrmoCallbackList *list,
                               IrmoObject *object);

#endif /* #ifndef IRMO_WORLD_OBJECT_CALLBACK_DATA_H */

