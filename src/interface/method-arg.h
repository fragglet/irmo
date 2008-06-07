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

#ifndef IRMO_INTERFACE_METHOD_ARG_H
#define IRMO_INTERFACE_METHOD_ARG_H

#include <irmo/interface.h>

// Maximum number of method arguments per method.  Imposed by
// the network protocol.

#define MAX_ARGUMENTS 256

// an argument to a method

struct _IrmoMethodArg {

        // Parent method that this argument belongs to.

	IrmoMethod *method;

        // Name of this method argument.

	char *name;

        // Index of this argument in the parent method's list of arguments.

	unsigned int index;

        // Type of value that this argument passes.

	IrmoValueType type;
};

/*!
 * Generate a hash of a @ref IrmoMethodArg.
 *
 * @param method_arg       The method argument.
 * @return                 The hash.
 */

uint32_t irmo_method_arg_hash(IrmoMethodArg *method_arg);

/*!
 * Free a @ref IrmoMethodArg.
 *
 * @param method_arg       The method argument to free.
 */

void _irmo_method_arg_free(IrmoMethodArg *arg);

#endif /* #ifndef IRMO_INTERFACE_METHOD_ARG_H */

