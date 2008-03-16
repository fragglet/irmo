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
// Introspection structure class, internal header.
//

#ifndef IRMO_BINDING_STRUCT_H
#define IRMO_BINDING_STRUCT_H

typedef struct _IrmoStruct IrmoStruct;

#include <algo/hashtable.h>
#include "struct-member.h"

/*!
 * A C structure.
 */

struct _IrmoStruct {

        /*! Name of this structure. */

        char *name;

        /*! Hash table of structure members, indexed by name. */

        IrmoHashTable *members;
};

/*!
 * Create a new @ref IrmoStruct.
 *
 * @param name         Name of the structure.
 * @return             Pointer to a new @ref IrmoStruct object.
 */

IrmoStruct *irmo_struct_new(char *name);

/*!
 * Get the member with the specified name.
 *
 * @param structure    The structure.
 * @param name         Name of the member to find.
 * @return             Pointer to the member, or NULL if it was not found.
 */

IrmoStructMember *irmo_struct_get_member(IrmoStruct *structure, char *name);

/*!
 * Add a new member to the given struct.
 *
 * @param structure    The structure.
 * @param member_name  Name of the new member.
 * @param offset       Offset of the member from the start of the structure,
 *                     in bytes.
 * @param member_size  Size of the member, in bytes.
 *
 * @return             Pointer to the new member.
 */

IrmoStructMember *irmo_struct_add_member(IrmoStruct *structure,
                                         char *member_name,
                                         unsigned long offset,
                                         unsigned long member_size);

#endif /* #ifndef IRMO_BINDING_STRUCT_H */

