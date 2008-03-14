//
// Copyright (C) 2008 Simon Howard
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
// Irmo structure binding, internal header
//

#ifndef IRMO_BINDING_BINDING_H
#define IRMO_BINDING_BINDING_H

#include "struct.h"
#include "struct-member.h"

/*!
 * Get the structure with the specified name.
 *
 * @param name         Name of the structure.
 * @return             Pointer to the @ref IrmoStruct object representing
 *                     the structure, or NULL if it has not been mapped
 *                     out.
 */

IrmoStruct *irmo_binding_get_struct(char *name);

#endif /* #ifndef IRMO_BINDING_BINDING_H */

