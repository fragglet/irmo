//
// Copyright (C) 2002-3 Simon Howard
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

#include <stdlib.h>
#include <assert.h>

#include <irmo.h>

// Build up a basic interface

IrmoInterface *build_interface(void)
{
        IrmoInterface *iface;
        IrmoMethod *method;
        IrmoClass *klass;
        IrmoClass *subclass;

        iface = irmo_interface_new();

        assert(iface != NULL);

        klass = irmo_interface_new_class(iface, "myclass", NULL);
        
        assert(klass != NULL);

        assert(irmo_class_new_variable(klass, "classvar",
                                       IRMO_TYPE_INT8) != NULL);

        subclass = irmo_interface_new_class(iface, "mysubclass", klass);

        assert(irmo_class_new_variable(subclass, "subclassvar",
                                       IRMO_TYPE_INT8) != NULL);

        method = irmo_interface_new_method(iface, "mymethod");

        assert(method != NULL);

        assert(irmo_method_new_argument(method, "argument",
                                        IRMO_TYPE_INT8) != NULL);

        return iface;
}

void test_build_interface(void)
{
        IrmoInterface *iface;

        iface = build_interface();

        irmo_interface_unref(iface);
}

// Check we cannot create duplicates with the same name

void test_duplicates(void)
{
        IrmoInterface *iface;
        IrmoClass *klass;
        IrmoClass *subclass;
        IrmoMethod *method;
        
        iface = build_interface();
        klass = irmo_interface_get_class(iface, "myclass");
        subclass = irmo_interface_get_class(iface, "mysubclass");
        method = irmo_interface_get_method(iface, "mymethod");

        assert(irmo_interface_new_class(iface, "myclass", NULL) == NULL);
        assert(irmo_interface_new_method(iface, "mymethod") == NULL);

        assert(irmo_class_new_variable(klass, "classvar", IRMO_TYPE_INT8) 
                == NULL);
        assert(irmo_method_new_argument(method, "argument", IRMO_TYPE_INT8) 
                == NULL);

        // Cannot create a variable in a subclass with the same name as the
        // superclass

        assert(irmo_class_new_variable(subclass, "classvar", IRMO_TYPE_INT8)
               == NULL);

        irmo_interface_unref(iface);
}

// Check counts

void test_counts(void)
{
        IrmoInterface *iface;
        IrmoClass *klass;
        IrmoClass *subclass;
        IrmoMethod *method;
        
        iface = build_interface();
        klass = irmo_interface_get_class(iface, "myclass");
        subclass = irmo_interface_get_class(iface, "mysubclass");
        method = irmo_interface_get_method(iface, "mymethod");

        assert(irmo_interface_num_classes(iface) == 2);
        assert(irmo_interface_num_methods(iface) == 1);
        assert(irmo_class_num_variables(klass) == 1);
        assert(irmo_class_num_variables(subclass) == 2);
        assert(irmo_method_num_arguments(method) == 1);

        irmo_interface_unref(iface);
}

// Test iterators

void test_class_iterator(void)
{
        IrmoIterator *iter;
        IrmoInterface *iface;
        IrmoClass *klass;
        char *name;
        int i;

        iface = build_interface();
        iter = irmo_interface_iterate_classes(iface);

        i = 0;

        while (irmo_iterator_has_more(iter)) {
                klass = irmo_iterator_next(iter);

                name = irmo_class_get_name(klass);
                assert(klass == irmo_interface_get_class(iface, name));
                ++i;
        }

        assert(i == 2);

        irmo_iterator_free(iter);

        irmo_interface_unref(iface);
}

void test_class_var_iterator(void)
{
        IrmoIterator *iter;
        IrmoInterface *iface;
        IrmoClass *klass;
        IrmoClassVar *var;
        char *name;
        int i;

        iface = build_interface();
        klass = irmo_interface_get_class(iface, "mysubclass");

        // Check, iterating parent variables
     
        iter = irmo_class_iterate_variables(klass, 1);

        i = 0;

        while (irmo_iterator_has_more(iter)) {
                var = irmo_iterator_next(iter);

                name = irmo_class_var_get_name(var);
                assert(var == irmo_class_get_variable(klass, name));
                ++i;
        }

        assert(i == 2);

        irmo_iterator_free(iter);

        // Check, not iterating parent variables

        iter = irmo_class_iterate_variables(klass, 0);

        i = 0;

        while (irmo_iterator_has_more(iter)) {
                var = irmo_iterator_next(iter);

                name = irmo_class_var_get_name(var);
                assert(var == irmo_class_get_variable(klass, name));
                ++i;
        }

        assert(i == 1);

        irmo_iterator_free(iter);

        irmo_interface_unref(iface);
}

void test_method_iterator(void)
{
        IrmoIterator *iter;
        IrmoInterface *iface;
        IrmoMethod *method;
        char *name;
        int i;

        iface = build_interface();
        iter = irmo_interface_iterate_methods(iface);

        i = 0;

        while (irmo_iterator_has_more(iter)) {
                method = irmo_iterator_next(iter);

                name = irmo_method_get_name(method);
                assert(method == irmo_interface_get_method(iface, name));
                ++i;
        }

        assert(i == 1);

        irmo_iterator_free(iter);

        irmo_interface_unref(iface);
}

void test_method_arg_iterator(void)
{
        IrmoIterator *iter;
        IrmoInterface *iface;
        IrmoMethod *method;
        IrmoMethodArg *arg;
        char *name;
        int i;

        iface = build_interface();
        method = irmo_interface_get_method(iface, "mymethod");
        iter = irmo_method_iterate_arguments(method);

        i = 0;

        while (irmo_iterator_has_more(iter)) {
                arg = irmo_iterator_next(iter);

                name = irmo_method_arg_get_name(arg);
                assert(arg == irmo_method_get_argument(method, name));
                ++i;
        }

        assert(i == 1);

        irmo_iterator_free(iter);

        irmo_interface_unref(iface);
}

void test_dump_and_load(void)
{
        IrmoInterface *iface;
        IrmoInterface *loaded_iface;
        void *buf;
        unsigned int buf_len;

        iface = build_interface();

        irmo_interface_dump(iface, &buf, &buf_len);

        loaded_iface = irmo_interface_load(buf, buf_len);

        assert(loaded_iface != NULL);
        assert(irmo_interface_hash(iface) == irmo_interface_hash(loaded_iface));
}

int main(int argc, char *argv[])
{
        test_build_interface();
        test_duplicates();
        test_counts();
        test_class_iterator();
        test_class_var_iterator();
        test_method_iterator();
        test_method_arg_iterator();
        test_dump_and_load();

        return 0;
}

