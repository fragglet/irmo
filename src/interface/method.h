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

#ifndef IRMO_INTERFACE_METHOD_H
#define IRMO_INTERFACE_METHOD_H

#include <irmo/interface.h>

#include "algo/algo.h"

// Maximum number of methods per interface.
// Limit imposed by the network protocol.

#define MAX_METHODS 256

// method declaration

struct _IrmoMethod {

        // Parent interface that this method belongs to.

	IrmoInterface *iface;

        // Name of this method.

	char *name;

        // Index of this method in the parent interface's list of methods.

	unsigned int index;
	
        // List of arguments to this method.

	IrmoMethodArg **arguments;
	unsigned int narguments;

        // Hash table for looking up method arguments by name.

	IrmoHashTable *argument_hash;
};

uint32_t irmo_method_hash(IrmoMethod *method);
void _irmo_method_free(IrmoMethod *method);

#endif /* #ifndef IRMO_INTERFACE_METHOD_H */

