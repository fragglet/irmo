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

#ifndef IRMO_INTERFACE_CLASS_H
#define IRMO_INTERFACE_CLASS_H

#include <irmo/interface.h>
#include "binding/binding.h"

// Maximum classes per interface.
// Limit imposed by the network protocol.

#define MAX_CLASSES 256

// class

struct _IrmoClass {

	// Interface that this class belongs to.

	IrmoInterface *iface;

	// Parent class or NULL if this is a base class and has no
        // parent.
	
	IrmoClass *parent_class;

        // Name of this class.

	char *name;

        // Index in the parent interface's list of classes.

	int index;
	
        // Array of member variables stored by each instance of this class.

	IrmoClassVar **variables;
	unsigned int nvariables;
	
        // Hash table to look up variables by name.

	IrmoHashTable *variable_hash;

        // Type of C structure that objects of this structure are bound to.

        IrmoStruct *structure;
};

uint32_t irmo_class_hash(IrmoClass *klass);
void _irmo_class_free(IrmoClass *klass);

/*!
 * Set default bindings for the specified class (bind to a structure
 * with the same name as the class, if one exists)
 *
 * @param klass           The class.
 * @return                Non-zero if the class was successfully 
 *                        bound to a structure, or zero if none
 *                        exists.
 */

int irmo_class_get_default_binding(IrmoClass *klass);

#endif /* #ifndef IRMO_INTERFACE_CLASS_H */

