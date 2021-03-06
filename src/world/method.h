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

#ifndef IRMO_WORLD_METHOD_H
#define IRMO_WORLD_METHOD_H

#include <irmo/method.h>

#include "interface/interface.h"

#include "object.h"

struct _IrmoMethodData {

        // Pointer to the method to invoke.

	IrmoMethod *method;

        // Client that invoked the method, or NULL if it was invoked
        // locally.

	IrmoClient *src;

        // Array of method arguments.

	IrmoValue *args;
};

/*!
 * Internal function to invoke a method.
 *
 * @param world               The world on which to invoke the method.
 * @param data                Method data (method arguments).
 */

void irmo_method_internal_call(IrmoWorld *world, IrmoMethodData *data);

#endif /* #ifndef IRMO_WORLD_METHOD_H */

