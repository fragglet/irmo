//
// Copyright (C) 2007 Simon Howard
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

#include <irmo.h>

static IrmoInterface *gen_interface(void)
{
        IrmoInterface *iface;
        IrmoClass *klass;
        IrmoClass *subclass;

        iface = irmo_interface_new();

        klass = irmo_interface_new_class(iface, "myclass", NULL);

        irmo_class_new_variable(klass, "myint", IRMO_TYPE_INT32);
        irmo_class_new_variable(klass, "myint2", IRMO_TYPE_INT32);
        irmo_class_new_variable(klass, "myint8", IRMO_TYPE_INT8);
        irmo_class_new_variable(klass, "myint16", IRMO_TYPE_INT16);
        irmo_class_new_variable(klass, "myint32", IRMO_TYPE_INT32);
        irmo_class_new_variable(klass, "mystring", IRMO_TYPE_STRING);

        subclass = irmo_interface_new_class(iface, "mysubclass", klass);

        irmo_class_new_variable(subclass, "myint3", IRMO_TYPE_INT32);

        return iface;
}

static IrmoWorld *gen_world(IrmoInterface **save_iface)
{
        IrmoInterface *iface;
        IrmoWorld *world;

        iface = gen_interface();
        world = irmo_world_new(iface);
        irmo_interface_unref(iface);

        if (save_iface != NULL) {
                *save_iface = iface;
        }

        return world;
}

// Test irmo_world_new

void test_world_new(void)
{
        IrmoInterface *iface;
        IrmoWorld *world;

        iface = gen_interface();

        world = irmo_world_new(iface);

        assert(world != NULL);
        assert(irmo_world_get_interface(world) == iface);

        irmo_interface_unref(iface);

        irmo_world_unref(world);
}

// Test object creation

void test_object_new(void)
{
        IrmoInterface *iface;
        IrmoWorld *world;
        IrmoClass *superclass;
        IrmoClass *subclass;
        IrmoObject *obj;
        IrmoObjectID id;

        // Create the world

        world = gen_world(&iface);

        superclass = irmo_interface_get_class(iface, "myclass");
        subclass = irmo_interface_get_class(iface, "mysubclass");
       
        // Create an object

        obj = irmo_object_new(world, "mysubclass");
        assert(obj != NULL);

        // Test object class structure

        assert(irmo_object_is_a(obj, "myclass"));
        assert(irmo_object_is_a(obj, "mysubclass"));
        assert(irmo_object_is_a2(obj, superclass));
        assert(irmo_object_is_a2(obj, subclass));

        assert(!strcmp(irmo_object_get_class(obj), "mysubclass"));
        assert(irmo_object_get_class_obj(obj) == subclass);

        // Test object id

        id = irmo_object_get_id(obj);

        assert(irmo_world_get_object_for_id(world, id) == obj);

        irmo_world_unref(world);
}

// Test object destroy

void test_object_destroy(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        IrmoObjectID obj_id;

        // Build world with an object

        world = gen_world(NULL);
        obj = irmo_object_new(world, "mysubclass");

        assert(obj != NULL);

        obj_id = irmo_object_get_id(obj);

        assert(irmo_world_get_object_for_id(world, obj_id) == obj);

        irmo_object_destroy(obj);

        assert(irmo_world_get_object_for_id(world, obj_id) == NULL);

        irmo_world_unref(world);
}

// Test object get_data/set_data.

void test_object_data(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int myint = 1;

        world = gen_world(NULL);

        obj = irmo_object_new(world, "myclass");

        assert(irmo_object_get_data(obj) == NULL);

        irmo_object_set_data(obj, &myint);

        assert(irmo_object_get_data(obj) == &myint);

        irmo_world_unref(world);
}

const unsigned int test_numbers_8[] = {
        0, 9, 28, 34, 39, 46, 49, 62, 67, 87, 111, 130, 132, 142, 152, 177,
        201, 208, 232, 241, 253, 255
};

const unsigned int test_numbers_16[] = {
        0, 1783, 5122, 7338, 8194, 8197, 9225, 16170, 16192, 16660, 31758,
        33441, 39158, 39821, 40674, 42536, 44633, 47650, 61042, 62437,
        63099, 65535
};

const unsigned int test_numbers_32[] = {
        0, 164502169, 177534637, 194570440, 304167745, 548619939, 1557918664,
        1777461450U, 2027481165U, 2190087792U, 2566040083U, 2983834323U,
        3096492259U, 3177207459U, 3415919482U, 3627798883U, 3749109130U,
        3873357266U, 4033938613U, 4082123822U, 4206187473U, 4294967295U
};

// Test object get/set

void test_object_get_set(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int i;

        world = gen_world(NULL);

        obj = irmo_object_new(world, "myclass");

        // Test setting int values

        for (i=0; i<22; ++i) {
                irmo_object_set_int(obj, "myint8", test_numbers_8[i]);
                irmo_object_set_int(obj, "myint16", test_numbers_16[i]);
                irmo_object_set_int(obj, "myint32", test_numbers_32[i]);

                assert(irmo_object_get_int(obj, "myint8") == test_numbers_8[i]);
                assert(irmo_object_get_int(obj, "myint16") == test_numbers_16[i]);
                assert(irmo_object_get_int(obj, "myint32") == test_numbers_32[i]);
        }

        // Test setting string values

        irmo_object_set_string(obj, "mystring", "hello, world");
        assert(!strcmp(irmo_object_get_string(obj, "mystring"),
                       "hello, world"));

        irmo_object_set_string(obj, "mystring", "");
        assert(!strcmp(irmo_object_get_string(obj, "mystring"), ""));

        irmo_world_unref(world);
}

void test_world_callbacks(void)
{
        // ... todo
}

void test_object_callbacks(void)
{
        // ... todo
}

int main(int argc, char *argv[])
{
        test_world_new();
        test_object_new();
        test_object_destroy();
        test_object_data();
        test_object_get_set();
        test_world_callbacks();
        test_object_callbacks();

        return 0;
}



