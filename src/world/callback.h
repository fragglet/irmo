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

#ifndef IRMO_INTERNAL_CALLBACK_H
#define IRMO_INTERNAL_CALLBACK_H

typedef struct _IrmoCallbackData IrmoCallbackData;

#include <irmo/callback.h>

#include "interface/interface.h"
#include "net/client.h"

#include "object.h"

struct _IrmoCallback {
	IrmoSListEntry **list;                 // callback list this belongs to
	void *func;
        void *user_data;
	IrmoSListEntry *destroy_callbacks;
};

struct _IrmoCallbackData {

	// Class this callback data is being used for watching
	// if this is NULL, it is for the global callback data
	// for a world, used to watch all objects.

	IrmoClass *objclass;

	// callback data object for the parent class
	
	IrmoCallbackData *parent_data;

	// callbacks for watching for creations of new objects
	// this is only for the class callbackdata - redundant
	// in object callbacks

	IrmoSListEntry *new_callbacks;
	
	// callbacks for if any variable is changed

	IrmoSListEntry *class_callbacks;

	// callbacks for if a particular variable is changed
	// this is redundant if objclass == NULL
	
	IrmoSListEntry **variable_callbacks;

	// callbacks called when object is destroyed
	
	IrmoSListEntry *destroy_callbacks;
};

// generalised callback list functions

IrmoCallback *irmo_callbacklist_add(IrmoSListEntry **list, void *func, 
				    void *user_data);
void irmo_callbacklist_free(IrmoSListEntry **list);

IrmoCallbackData *irmo_callbackdata_new(IrmoClass *objclass, 
					IrmoCallbackData *parent_data);
void irmo_callbackdata_free(IrmoCallbackData *data);
void irmo_callbackdata_raise(IrmoCallbackData *data,
			     IrmoObject *object,
                             unsigned int variable_index);
void irmo_callbackdata_raise_destroy(IrmoCallbackData *data, 
				     IrmoObject *object);
void irmo_callbackdata_raise_new(IrmoCallbackData *data, 
				 IrmoObject *object);

#endif /* #ifndef IRMO_INTERNAL_CALLBACK_H */

