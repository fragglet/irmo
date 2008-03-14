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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <irmo/binding.h>
#include "binding/binding.h"

struct mystruct {
        int8_t myint8;
        int16_t myint16;
        int32_t myint32;
        char *mystring;
};

// Test the structure member macros in binding.h

static void test_member_macros(void)
{
        struct mystruct *nothing = NULL;

        assert(irmo_sizeof_member(struct mystruct, myint8) == 1);
        assert(irmo_sizeof_member(struct mystruct, myint16) == 2);
        assert(irmo_sizeof_member(struct mystruct, myint32) == 4);
        assert(irmo_sizeof_member(struct mystruct, mystring) == sizeof(char *));

        assert(irmo_offsetof(struct mystruct, myint8) == 0);
        assert(irmo_offsetof(struct mystruct, myint16) == (unsigned long) &nothing->myint16);
        assert(irmo_offsetof(struct mystruct, myint32) == (unsigned long) &nothing->myint32);
        assert(irmo_offsetof(struct mystruct, mystring) == (unsigned long) &nothing->mystring);
}

static void test_map_bindings(void)
{
        irmo_map_struct(struct mystruct, myint8);
        irmo_map_struct(struct mystruct, myint16);
        irmo_map_struct(struct mystruct, myint32);
        irmo_map_struct(struct mystruct, mystring);

        // Duplicates should print a debug message

        irmo_map_struct(struct mystruct, mystring);
}

static void test_struct(void)
{
        IrmoStruct *structure;

        structure = irmo_binding_get_struct("struct mystruct");

        assert(structure != NULL);

        assert(irmo_binding_get_struct("mystruct") == NULL);
        assert(irmo_binding_get_struct("asdf") == NULL);

        // Test getting members

        assert(irmo_struct_get_member(structure, "myint8") != NULL);
        assert(irmo_struct_get_member(structure, "myint16") != NULL);
        assert(irmo_struct_get_member(structure, "myint32") != NULL);
        assert(irmo_struct_get_member(structure, "mystring") != NULL);
        assert(irmo_struct_get_member(structure, "asdf") == NULL);
}

static void test_struct_member(void)
{
        IrmoStruct *structure;
        IrmoStructMember *myint32, *mystring;
        struct mystruct *nothing = NULL;

        structure = irmo_binding_get_struct("struct mystruct");
        myint32 = irmo_struct_get_member(structure, "myint32");
        mystring = irmo_struct_get_member(structure, "mystring");

        assert(myint32 != NULL);
        assert(mystring != NULL);

        assert(strcmp(myint32->name, "myint32") == 0);
        assert(myint32->size == 4);
        assert(myint32->offset == (unsigned long) &nothing->myint32);

        assert(strcmp(mystring->name, "mystring") == 0);
        assert(mystring->size == sizeof(char *));
        assert(mystring->offset == (unsigned long) &nothing->mystring);
}

int main(int argc, char *argv[])
{
        test_member_macros();
        test_map_bindings();
        test_struct();
        test_struct_member();

        return 0;
}


