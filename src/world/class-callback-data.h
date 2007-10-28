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

//
// Callbacks
//

#ifndef IRMO_WORLD_CLASS_CALLBACK_DATA_H
#define IRMO_WORLD_CLASS_CALLBACK_DATA_H

typedef struct _ClassCallbackData ClassCallbackData;

#include "interface/class.h"
#include "object-callback-data.h"

struct _ClassCallbackData {

        // Class that this applies to.

        IrmoClass *klass;

	// Callback data object for the parent class
	
	ClassCallbackData *parent_data;

	// Callbacks for watching for creation of new objects.

	IrmoCallbackList new_callbacks;
	
        // Other callback data

        ObjectCallbackData object_callbacks;
};

void irmo_class_callback_init(ClassCallbackData *data,
                              ClassCallbackData *parent_data,
                              IrmoClass *klass);
void irmo_class_callback_free(ClassCallbackData *data);

void irmo_class_callback_raise(ClassCallbackData *data,
                               IrmoObject *object,
                               unsigned int variable_index);
void irmo_class_callback_raise_destroy(ClassCallbackData *data, 
                                       IrmoObject *object);
void irmo_class_callback_raise_new(ClassCallbackData *data, 
                                   IrmoObject *object);

IrmoCallback *irmo_class_callback_watch(ClassCallbackData *data,
                                        IrmoClass *klass,
                                        char *variable_name,
                                        IrmoVarCallback func,
                                        void *user_data);
IrmoCallback *irmo_class_callback_watch_destroy(ClassCallbackData *data,
                                                IrmoObjCallback func,
                                                void *user_data);
IrmoCallback *irmo_class_callback_watch_new(ClassCallbackData *data,
                                            IrmoObjCallback func,
                                            void *user_data);

#endif /* #ifndef IRMO_WORLD_CLASS_CALLBACK_DATA_H */

