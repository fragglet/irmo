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
// Introspection structure class.
//

#include "arch/sysheaders.h"
#include "base/alloc.h"
#include "base/assert.h"

#include "struct.h"

IrmoStruct *irmo_struct_new(char *name)
{
        IrmoStruct *structure;

        structure = irmo_new0(IrmoStruct, 1);
        structure->name = name;
        structure->members = irmo_hash_table_new(irmo_string_hash,
                                                     irmo_string_equal);

        irmo_alloc_assert(structure->members != NULL);

        return structure;
}

IrmoStructMember *irmo_struct_get_member(IrmoStruct *structure, char *name)
{
        return irmo_hash_table_lookup(structure->members, name);
}

IrmoStructMember *irmo_struct_add_member(IrmoStruct *structure,
                                         char *member_name,
                                         unsigned long offset,
                                         unsigned long member_size)
{
        IrmoStructMember *member;

        // Check for duplicates

        irmo_return_val_if_fail(
                irmo_struct_get_member(structure, member_name) == NULL,
                NULL
        );

        // Create the new member

        member = irmo_struct_member_new(member_name, offset, member_size);
        irmo_alloc_assert(irmo_hash_table_insert(structure->members,
                                                 member_name,
                                                 member));

        return member;
}


