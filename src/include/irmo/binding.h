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

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Structure mapping.  This module contains macros for mapping out 
 * C structures, so that the structures can be automatically updated.
 *
 * @addtogroup binding
 * \{
 */


#ifndef IRMO_BINDING_H
#define IRMO_BINDING_H

#include <irmo/types.h>

/*!
 * Find the offset of a structure member from the start of the 
 * structure.
 *
 * @param structure       The structure.
 * @param member          The member.
 * @return                Offset of the member from the start of the 
 *                        structure, in bytes.
 */

#define irmo_offsetof(structure, member)                             \
        ( (unsigned long) &(((structure *) 0)->member) )

/*!
 * Find the size of a structure member.
 *
 * @param structure       The structure.
 * @param member          The member.
 * @return                Size of the member, in bytes.
 */

#define irmo_sizeof_member(structure, member)                        \
        sizeof( ((structure *) 0)->member )

/*!
 * Map out a particular structure member.
 *
 * @param structure       The structure to map.
 * @param member          The structure member to map.
 */

#define irmo_map_struct(structure, member)                             \
        irmo_binding_add_member(#structure, #member,                   \
                                irmo_offsetof(structure, member),      \
                                irmo_sizeof_member(structure, member))

/*!
 * Add binding information for the specified structure member.
 * It is normally easier to use the @ref irmo_map_struct macro.
 *
 * @param struct_name     Name of the structure.
 * @param member_name     Name of the structure member.
 * @param offset          Offset in bytes from the start of the structure.
 * @param member_size     Size of the member, in bytes.
 */

void irmo_binding_add_member(char *struct_name, char *member_name,
                             unsigned long offset, unsigned long member_size);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_BINDING_H */

