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

#ifndef IRMO_BINDING_STRUCT_MEMBER_H
#define IRMO_BINDING_STRUCT_MEMBER_H

typedef struct _IrmoStructMember IrmoStructMember;

/*! 
 * A particular member of a C structure.
 */

struct _IrmoStructMember {

        /*! String containing the name of this member. */

        char *name;

        /*! Offset of this member from the start of the struct, in bytes. */

        unsigned long offset;

        /*! Size of this member, in bytes. */

        unsigned long size;
};

/*!
 * Create a new structure member.
 *
 * @param name           Name of the new member.
 * @param offset         Offset of the member from the start of the structure,
 *                       in bytes.
 * @param size           Size of the member, in bytes.
 */

IrmoStructMember *irmo_struct_member_new(char *name, unsigned long offset,
                                         unsigned long size);

/*!
 * Get the value of a structure member (string type).
 *
 * @param member         The structure member.
 * @param structure      Pointer to the structure.
 * @return               The value.
 */

char *irmo_struct_member_get_string(IrmoStructMember *member, void *structure);

/*!
 * Set the value of a structure member (string type).
 *
 * @param member         The structure member.
 * @param structure      Pointer to the structure.
 * @param value          The value value to set.
 */

void irmo_struct_member_set_string(IrmoStructMember *member, void *structure,
                                   char *value);

/*!
 * Get the value of a structure member (integer type).
 *
 * @param member         The structure member.
 * @param structure      Pointer to the structure.
 * @return               The value.
 */

unsigned int irmo_struct_member_get_int(IrmoStructMember *member,
                                        void *structure);

/*!
 * Set the value of a structure member (integer type).
 *
 * @param member         The structure member.
 * @param structure      Pointer to the structure.
 * @param value          The value to set.
 */

void irmo_struct_member_set_int(IrmoStructMember *member, void *structure,
                                unsigned int value);

#endif /* #ifndef IRMO_BINDING_STRUCT_MEMBER_H */

