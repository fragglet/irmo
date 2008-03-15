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
// Irmo structure binding (introspection).
//

#include "arch/sysheaders.h"
#include "base/util.h"

#include "binding.h"

static IrmoHashTable *mapped_structs = NULL;

void irmo_binding_add_member(char *struct_name, char *member_name,
                             unsigned long offset, unsigned long member_size)
{
        IrmoStruct *mapped_struct;

        irmo_return_if_fail(struct_name != NULL);
        irmo_return_if_fail(member_name != NULL);
        irmo_return_if_fail(member_size != 0);

        // First member mapped?

        if (mapped_structs == NULL) {

                // Create the structs hash table.

                mapped_structs = irmo_hash_table_new(irmo_string_hash,
                                                     irmo_string_equal);
                mapped_struct = NULL;
        } else {

                // Find the structure we are mapping.

                mapped_struct = irmo_hash_table_lookup(mapped_structs,
                                                       struct_name);
        }

        // First member of this structure to be mapped?
        // Create a new struct and add it to the list.

        if (mapped_struct == NULL) {

                mapped_struct = irmo_struct_new(struct_name);
                irmo_hash_table_insert(mapped_structs, struct_name, 
                                       mapped_struct);
        }

        // Add the new member to the struct.

        irmo_struct_add_member(mapped_struct, member_name,
                               offset, member_size);
}

IrmoStruct *irmo_binding_get_struct(char *name)
{
        return irmo_hash_table_lookup(mapped_structs, name);
}

