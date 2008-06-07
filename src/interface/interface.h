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

#ifndef IRMO_INTERFACE_INTERFACE_H
#define IRMO_INTERFACE_INTERFACE_H

#include <irmo/interface.h>

#include "algo/algo.h"

#include "class.h"
#include "class-var.h"
#include "method.h"
#include "method-arg.h"

struct _IrmoInterface {

        // Number of references to this interface.  When this reaches 
        // zero, the interface will be destroyed.

	int refcount;
	
	// List of classes that are part of this interface.
	
	IrmoClass **classes;
	unsigned int nclasses;

        // Hash table to look up classes by name.

	IrmoHashTable *class_hash;

	// List of methods that are part of this interface.
	
	IrmoMethod **methods;
	unsigned int nmethods;

        // Hash table to look up methods by name.

	IrmoHashTable *method_hash;
};

/*!
 * Generate a hash of a @ref IrmoInterface.
 *
 * @param iface            The interface.
 * @return                 The hash of the interface.
 */

uint32_t irmo_interface_hash(IrmoInterface *iface);

/*!
 * Free a @ref IrmoInterface.
 *
 * @param iface            The interface to free.
 */

void _irmo_interface_free(IrmoInterface *iface);

#endif /* #ifndef IRMO_INTERFACE_INTERFACE_H */

