//
// Copyright (C) 2009 Simon Howard
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

static IrmoInterface *test_interface;

// --- Test callback of type IrmoObjCallback ---

static IrmoObject *obj_callback_obj;
static void *obj_callback_user_data;

static void obj_callback(IrmoObject *obj, void *user_data)
{
        obj_callback_obj = obj;
        obj_callback_user_data = user_data;
}

static void obj_callback_clear(void)
{
        obj_callback_obj = NULL;
        obj_callback_user_data = NULL;
}

static int obj_callback_check(IrmoObject *obj, void *user_data)
{
        return obj_callback_obj == obj
            && obj_callback_user_data == user_data;
}

// --- Test callback of type IrmoVarCallback ---

static IrmoObject *var_callback_obj;
static IrmoClassVar *var_callback_var;
static void *var_callback_user_data;

static void var_callback(IrmoObject *obj, IrmoClassVar *var, void *user_data)
{
        var_callback_obj = obj;
        var_callback_var = var;
        var_callback_user_data = user_data;
}

static void var_callback_clear(void)
{
        var_callback_obj = NULL;
        var_callback_var = NULL;
        var_callback_user_data = NULL;
}

static int var_callback_check(IrmoObject *obj, char *varname, void *user_data)
{
        return var_callback_obj == obj
            && var_callback_user_data == user_data
            && !strcmp(irmo_class_var_get_name(var_callback_var), varname);
}

static IrmoInterface *gen_interface(void)
{
        IrmoInterface *iface;
        IrmoClass *klass;
        IrmoClass *subclass;

        iface = irmo_interface_new();

        klass = irmo_interface_new_class(iface, "myclass", NULL);

        irmo_class_new_variable(klass, "myint", IRMO_TYPE_INT32);

        subclass = irmo_interface_new_class(iface, "mysubclass", klass);

        irmo_class_new_variable(subclass, "myint2", IRMO_TYPE_INT32);

        return iface;
}

//
// Object watching tests
//
// These tests check that callbacks set to watch events occurring on
// specific objects are invoked correctly.
//

// Test watching for a specific object being destroyed.

void test_object_watch_destroy(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Create an object and set a watch.

        obj = irmo_object_new(world, "myclass");

        irmo_object_watch_destroy(obj, obj_callback, &dummy);

        // Destroy the object and check the callback was invoked.

        obj_callback_clear();

        irmo_object_destroy(obj);

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

// Test watching for a specific variable of a specific object being changed.

void test_object_watch_variable(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Create an object and set a watch.

        obj = irmo_object_new(world, "myclass");

        irmo_object_watch(obj, "myint", var_callback, &dummy);

        // Destroy the object and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        irmo_world_unref(world);
}

// Test watching for any variable of a specific object being changed.

void test_object_watch_all_variables(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Create an object and set a watch.

        obj = irmo_object_new(world, "myclass");

        irmo_object_watch(obj, NULL, var_callback, &dummy);

        // Change the variable and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        irmo_world_unref(world);
}

//
// World watch tests - basic
//
// These test the basic functionality of watching for events relating to
// objects of a specific class.
//

// Test watching for an object of a specific class being instantiated.

void test_world_watch_new(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set a watch.

        irmo_world_watch_new(world, "myclass", obj_callback, &dummy);

        // Create the object and check the callback was invoked.

        obj_callback_clear();

        obj = irmo_object_new(world, "myclass");

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

// Test watching for changes to a specific variable of all objects of
// a specific class.

void test_world_watch_variable(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set the watch.

        irmo_world_watch_class(world, "myclass", "myint",
                               var_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "myclass");

        // Change the variable and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        irmo_world_unref(world);
}

// Test watching for changes to any variable of all objects of
// a specific class.

void test_world_watch_all_variables(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set the watch.

        irmo_world_watch_class(world, "myclass", NULL,
                               var_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "myclass");

        // Change the variable and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        irmo_world_unref(world);
}

// Test watching for when objects of a specific class are destroyed.

void test_world_watch_destroy(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set a watch.

        irmo_world_watch_destroy(world, "myclass", obj_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "myclass");

        // Destroy the object and check the callback was invoked.

        obj_callback_clear();

        irmo_object_destroy(obj);

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

//
// World watch tests - subclassing
//
// These tests are identical to the basic tests (above), but check
// that watches also apply to subclasses of the class being watched.
//

// Test watching for an object of a specific class being instantiated,
// and that subclasses are also included.

void test_world_subclass_watch_new(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set a watch.

        irmo_world_watch_new(world, "myclass", obj_callback, &dummy);

        // Create the object and check the callback was invoked.

        obj_callback_clear();

        obj = irmo_object_new(world, "mysubclass");

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

// Test watching for changes to a specific variable of all objects of
// a specific class, and that subclasses are included.

void test_world_subclass_watch_variable(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set the watch.

        irmo_world_watch_class(world, "myclass", "myint",
                               var_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "mysubclass");

        // Change the variable and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        irmo_world_unref(world);
}

// Test watching for changes to any variable of all objects of
// a specific class, and that subclasses are included.

void test_world_subclass_watch_all_variables(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set the watch.

        irmo_world_watch_class(world, "myclass", NULL,
                               var_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "mysubclass");

        // Change the variable and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        // Test again, for "myint2".
        // The myint2 variable only exists in the subclass, but the watch for
        // the parent class should still trigger the callback.

        var_callback_clear();

        irmo_object_set_int(obj, "myint2", 100);

        assert(var_callback_check(obj, "myint2", &dummy));

        irmo_world_unref(world);
}

// Test watching for when objects of a specific class are destroyed,
// and that subclasses are included.

void test_world_subclass_watch_destroy(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set a watch.

        irmo_world_watch_destroy(world, "myclass", obj_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "mysubclass");

        // Destroy the object and check the callback was invoked.

        obj_callback_clear();

        irmo_object_destroy(obj);

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

//
// World watch tests - any class
//
// These test the watching for events relating to all objects in the
// world (any class)
//

// Test watching for an object of a specific class being instantiated.

void test_world_anyclass_watch_new(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set a watch.

        irmo_world_watch_new(world, NULL, obj_callback, &dummy);

        // Create the object and check the callback was invoked.

        obj_callback_clear();

        obj = irmo_object_new(world, "myclass");

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

// Test watching for changes to any variable of all objects of
// a specific class.

void test_world_anyclass_watch_all_variables(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set the watch.

        irmo_world_watch_class(world, NULL, NULL, var_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "myclass");

        // Change the variable and check the callback was invoked.

        var_callback_clear();

        irmo_object_set_int(obj, "myint", 100);

        assert(var_callback_check(obj, "myint", &dummy));

        irmo_world_unref(world);
}

// Test watching for when objects of a specific class are destroyed.

void test_world_anyclass_watch_destroy(void)
{
        IrmoWorld *world;
        IrmoObject *obj;
        int dummy;

        world = irmo_world_new(test_interface);

        // Set a watch.

        irmo_world_watch_destroy(world, NULL, obj_callback, &dummy);

        // Create the test object.

        obj = irmo_object_new(world, "myclass");

        // Destroy the object and check the callback was invoked.

        obj_callback_clear();

        irmo_object_destroy(obj);

        assert(obj_callback_check(obj, &dummy));

        irmo_world_unref(world);
}

int main(int argc, char *argv[])
{
        test_interface = gen_interface();

        test_object_watch_destroy();
        test_object_watch_variable();
        test_object_watch_all_variables();

        test_world_watch_new();
        test_world_watch_variable();
        test_world_watch_all_variables();
        test_world_watch_destroy();

        test_world_subclass_watch_new();
        test_world_subclass_watch_variable();
        test_world_subclass_watch_all_variables();
        test_world_subclass_watch_destroy();

        test_world_anyclass_watch_new();
        test_world_anyclass_watch_all_variables();
        test_world_anyclass_watch_destroy();

        return 0;
}



