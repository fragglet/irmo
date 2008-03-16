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
// Introspection structure member class.
//

#include "arch/sysheaders.h"
#include "base/util.h"

#include "struct-member.h"

#define MEMBER_PTR(member, structure)                                \
        (void *) (((int8_t *) structure) + member->offset)

IrmoStructMember *irmo_struct_member_new(char *name, unsigned long offset,
                                         unsigned long size)
{
        IrmoStructMember *member;

        member = irmo_new0(IrmoStructMember, 1);
        member->name = name;
        member->offset = offset;
        member->size = size;

        return member;
}

char *irmo_struct_member_get_string(IrmoStructMember *member, void *structure)
{
        char **member_value;

        irmo_return_val_if_fail(member->size == sizeof(char *), NULL);

        member_value = MEMBER_PTR(member, structure);

        return *member_value;
}

void irmo_struct_member_set_string(IrmoStructMember *member, void *structure,
                                   char *value)
{
        char **member_value;

        irmo_return_if_fail(member->size == sizeof(char *));

        member_value = MEMBER_PTR(member, structure);

        *member_value = value;
}

unsigned int irmo_struct_member_get_int(IrmoStructMember *member,
                                        void *structure)
{
        void *member_value;

        member_value = MEMBER_PTR(member, structure);

        switch (member->size) {
                case sizeof(uint8_t):
                        return *((uint8_t *) member_value);

                case sizeof(uint16_t):
                        return *((uint16_t *) member_value);

                case sizeof(uint32_t):
                        return *((uint32_t *) member_value);

                default:
                        irmo_warning_message(
                                "irmo_struct_member_get_int",
                                "Structure member '%s' has unsupported "
                                "size: '%i'", member->size);
                        return 0;
        }
}

void irmo_struct_member_set_int(IrmoStructMember *member, void *structure,
                                unsigned int value)
{
        void *member_value;

        member_value = MEMBER_PTR(member, structure);

        switch (member->size) {
                case sizeof(uint8_t):
                        *((uint8_t *) member_value) = value;
                        break;

                case sizeof(uint16_t):
                        *((uint16_t *) member_value) = value;
                        break;

                case sizeof(uint32_t):
                        *((uint32_t *) member_value) = value;
                        break;

                default:
                        irmo_warning_message(
                                "irmo_struct_member_set_int",
                                "Structure member '%s' has unsupported "
                                "size: '%i'", member->size);
                        break;
        }
}

