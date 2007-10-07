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

#ifndef IRMO_BASE_CALLBACK_H
#define IRMO_BASE_CALLBACK_H

#include <irmo/callback.h>

#include "algo/slist.h"

typedef IrmoSListEntry *IrmoCallbackList;

struct _IrmoCallback {
	IrmoCallbackList *list;              // callback list this belongs to
	void *func;
        void *user_data;
	IrmoCallbackList destroy_callbacks;
};

// generalised callback list functions

IrmoCallback *irmo_callback_list_add(IrmoCallbackList *list,
                                     void *func, void *user_data);
void irmo_callback_list_free(IrmoCallbackList *list);

#endif /* #ifndef IRMO_BASE_CALLBACK_H */

