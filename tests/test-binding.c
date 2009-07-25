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
#include <inttypes.h>

#include <irmo/binding.h>
#include "binding/binding.h"

struct mystruct {
        uint8_t myint8;
        uint16_t myint16;
        uint32_t myint32;
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

static void test_struct_member_get_set_int(void)
{
        struct mystruct s;
        IrmoStruct *structure;
        IrmoStructMember *myint8, *myint16, *myint32;

        structure = irmo_binding_get_struct("struct mystruct");

        // Test myint8

        myint8 = irmo_struct_get_member(structure, "myint8");

        s.myint8 = 0x53;
        assert(irmo_struct_member_get_int(myint8, &s) == 0x53);
        irmo_struct_member_set_int(myint8, &s, 0x26);
        assert(s.myint8 == 0x26);
        
        // Test myint16

        myint16 = irmo_struct_get_member(structure, "myint16");

        s.myint16 = 0x9a8f;
        assert(irmo_struct_member_get_int(myint16, &s) == 0x9a8f);
        irmo_struct_member_set_int(myint16, &s, 0x42ce);
        assert(s.myint16 == 0x42ce);
        
        // Test myint32

        myint32 = irmo_struct_get_member(structure, "myint32");

        s.myint32 = 0x32c2e293;
        assert(irmo_struct_member_get_int(myint32, &s) == 0x32c2e293);
        irmo_struct_member_set_int(myint32, &s, 0x12ca3b38);
        assert(s.myint32 == 0x12ca3b38);
}

static void test_struct_member_get_set_string(void)
{
        struct mystruct s;
        IrmoStruct *structure;
        IrmoStructMember *mystring;
        char *value1, *value2;

        structure = irmo_binding_get_struct("struct mystruct");

        value1 = "test value 1";
        value2 = "test value 2";

        mystring = irmo_struct_get_member(structure, "mystring");

        s.mystring = value1;
        assert(irmo_struct_member_get_string(mystring, &s) == value1);
        s.mystring = NULL;
        assert(irmo_struct_member_get_string(mystring, &s) == NULL);

        irmo_struct_member_set_string(mystring, &s, value2);
        assert(s.mystring == value2);
        irmo_struct_member_set_string(mystring, &s, NULL);
        assert(s.mystring == NULL);
}

int main(int argc, char *argv[])
{
        test_member_macros();
        test_map_bindings();
        test_struct();
        test_struct_member();
        test_struct_member_get_set_int();
        test_struct_member_get_set_string();

        return 0;
}


