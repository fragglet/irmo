//
// Copyright (C) 2007-8 Simon Howard
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
#include <inttypes.h>
#include <assert.h>

#include <irmo.h>

struct test_struct {
        uint32_t myint;
        uint32_t myint2;
        uint8_t myint8;
        uint16_t myint16;
        char *mystring;
};

struct test_struct_derived {
        struct test_struct parent;
        uint32_t myint3;
};

static int check_var_set_invoked;

static void test_callback_check_var_set(IrmoObject *obj,
                                        IrmoClassVar *var,
                                        void *user_data)
{
        struct test_struct *mystruct = user_data;

        check_var_set_invoked = 1;

        // The value in the struct should be set *before* the callback
        // is invoked:

        assert(mystruct->myint == 1234);
}

static void map_test_structs(void)
{
        irmo_map_struct(struct test_struct, myint);
        irmo_map_struct(struct test_struct, myint2);
        irmo_map_struct(struct test_struct, myint8);
        irmo_map_struct(struct test_struct, myint16);
        irmo_map_struct(struct test_struct, mystring);

        irmo_map_struct(struct test_struct_derived, myint3);
}

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

        irmo_interface_bind_class(iface, "myclass", "struct test_struct");

        subclass = irmo_interface_new_class(iface, "mysubclass", klass);

        irmo_class_new_variable(subclass, "myint3", IRMO_TYPE_INT32);

        irmo_interface_bind_class(iface, "mysubclass",
                                         "struct test_struct_derived");

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
        assert(irmo_world_num_objects(world) == 0);

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
        assert(irmo_world_num_objects(world) == 1);

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
        assert(irmo_world_num_objects(world) == 1);

        assert(obj != NULL);

        obj_id = irmo_object_get_id(obj);

        assert(irmo_world_get_object_for_id(world, obj_id) == obj);

        irmo_object_destroy(obj);

        assert(irmo_world_num_objects(world) == 0);

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

// Test object get/set (generic versions)

void test_object_get_set_generic(void)
{
        IrmoClass *myclass;
        IrmoClassVar *myint8, *myint16, *myint32, *mystring;
        IrmoInterface *iface;
        IrmoWorld *world;
        IrmoObject *obj;
        IrmoValue value, value2;
        int i;

        world = gen_world(NULL);

        iface = irmo_world_get_interface(world);
        myclass = irmo_interface_get_class(iface, "myclass");
        myint8 = irmo_class_get_variable(myclass, "myint8");
        myint16 = irmo_class_get_variable(myclass, "myint16");
        myint32 = irmo_class_get_variable(myclass, "myint32");
        mystring = irmo_class_get_variable(myclass, "mystring");

        obj = irmo_object_new(world, "myclass");

        // Test setting int values

        for (i=0; i<22; ++i) {
                value.i = test_numbers_8[i];
                irmo_object_set(obj, myint8, &value);
                value.i = test_numbers_16[i];
                irmo_object_set(obj, myint16, &value);
                value.i = test_numbers_32[i];
                irmo_object_set(obj, myint32, &value);

                // Read back

                irmo_object_get(obj, myint8, &value2);
                assert(value2.i == test_numbers_8[i]);
                irmo_object_get(obj, myint16, &value2);
                assert(value2.i == test_numbers_16[i]);
                irmo_object_get(obj, myint32, &value2);
                assert(value2.i == test_numbers_32[i]);
        }

        // Test setting string values

        value.s = "hello, world";
        irmo_object_set(obj, mystring, &value);
        irmo_object_get(obj, mystring, &value2);
        assert(!strcmp(value.s, "hello, world"));

        value.s = "";
        irmo_object_set(obj, mystring, &value);
        irmo_object_get(obj, mystring, &value2);
        assert(!strcmp(value.s, ""));

        irmo_world_unref(world);
}

void test_object_set_bindings(void)
{
        IrmoInterface *iface;
        IrmoWorld *world;
        struct test_struct mystruct;
        struct test_struct_derived mystruct2;
        IrmoObject *obj1;
        IrmoObject *obj2;

        iface = gen_interface();
        world = irmo_world_new(iface);
        irmo_interface_unref(iface);

        obj1 = irmo_object_new(world, "myclass");
        memset(&mystruct, 0, sizeof(mystruct));
        irmo_object_bind(obj1, &mystruct);

        obj2 = irmo_object_new(world, "mysubclass");
        memset(&mystruct2, 0, sizeof(mystruct2));
        irmo_object_bind(obj2, &mystruct2);

        irmo_object_watch(obj1, "myint", test_callback_check_var_set, &mystruct);
        check_var_set_invoked = 0;

        // Superclass test:

        irmo_object_set_int(obj1, "myint", 1234);
        irmo_object_set_int(obj1, "myint2", 4321);
        irmo_object_set_int(obj1, "myint8", 42);
        irmo_object_set_int(obj1, "myint16", 5678);
        irmo_object_set_string(obj1, "mystring", "hello world");

        assert(mystruct.myint == 1234);
        assert(mystruct.myint2 == 4321);
        assert(mystruct.myint8 == 42);
        assert(mystruct.myint16 == 5678);
        assert(!strcmp(mystruct.mystring, "hello world"));

        // Subclassing test:

        irmo_object_set_int(obj2, "myint", 6789);
        irmo_object_set_int(obj2, "myint16", 2266);
        irmo_object_set_int(obj2, "myint3", 9999);

        assert(mystruct2.parent.myint == 6789);
        assert(mystruct2.parent.myint16 == 2266);
        assert(mystruct2.myint3 == 9999);

        // Check that the callback was invoked:

        assert(check_var_set_invoked);

        irmo_world_unref(world);
}

void test_object_get_bindings(void)
{
        IrmoInterface *iface;
        IrmoWorld *world;
        struct test_struct mystruct;
        struct test_struct_derived mystruct2;
        IrmoObject *obj1;
        IrmoObject *obj2;

        iface = gen_interface();
        world = irmo_world_new(iface);
        irmo_interface_unref(iface);

        obj1 = irmo_object_new(world, "myclass");
        memset(&mystruct, 0, sizeof(mystruct));
        irmo_object_bind(obj1, &mystruct);

        obj2 = irmo_object_new(world, "mysubclass");
        memset(&mystruct2, 0, sizeof(mystruct2));
        irmo_object_bind(obj2, &mystruct2);

        irmo_object_watch(obj1, "myint", test_callback_check_var_set, &mystruct);
        check_var_set_invoked = 0;

        // Test superclass:

        mystruct.myint = 1234;
        mystruct.myint2 = 4321;
        mystruct.myint8 = 42;
        mystruct.myint16 = 5678;
        mystruct.mystring = "hello world";

        irmo_object_update(obj1);

        assert(irmo_object_get_int(obj1, "myint") == 1234);
        assert(irmo_object_get_int(obj1, "myint2") == 4321);
        assert(irmo_object_get_int(obj1, "myint8") == 42);
        assert(irmo_object_get_int(obj1, "myint16") == 5678);
        assert(!strcmp(irmo_object_get_string(obj1, "mystring"),
                       "hello world"));

        // Test subclass:

        mystruct2.parent.myint = 9876;
        mystruct2.parent.myint2 = 5555;
        mystruct2.myint3 = 3456;

        irmo_world_update(world);

        assert(irmo_object_get_int(obj2, "myint") == 9876);
        assert(irmo_object_get_int(obj2, "myint2") == 5555);
        assert(irmo_object_get_int(obj2, "myint3") == 3456);

        // Check that the callback was invoked:

        assert(check_var_set_invoked);

        irmo_world_unref(world);
}

void test_world_iterate(void)
{
        // TODO
}

int main(int argc, char *argv[])
{
        map_test_structs();

        test_world_new();
        test_object_new();
        test_object_destroy();
        test_object_data();
        test_object_get_set();
        test_object_get_set_generic();
        test_object_set_bindings();
        test_object_get_bindings();
        test_world_iterate();

        return 0;
}



