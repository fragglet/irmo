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

        iface = irmo_interface_new();

        assert(iface != NULL);

        klass = irmo_interface_new_class(iface, "myclass", NULL);
        
        assert(klass != NULL);

        assert(irmo_class_new_variable(klass, "classvar",
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
        IrmoMethod *method;
        
        iface = build_interface();
        klass = irmo_interface_get_class(iface, "myclass");
        method = irmo_interface_get_method(iface, "mymethod");

        assert(irmo_interface_new_class(iface, "myclass", NULL) == NULL);
        assert(irmo_interface_new_method(iface, "mymethod") == NULL);

        assert(irmo_class_new_variable(klass, "classvar", IRMO_TYPE_INT8) 
                == NULL);
        assert(irmo_method_new_argument(method, "argument", IRMO_TYPE_INT8) 
                == NULL);

        irmo_interface_unref(iface);
}

// Check counts

void test_counts(void)
{
        IrmoInterface *iface;
        IrmoClass *klass;
        IrmoMethod *method;
        
        iface = build_interface();
        klass = irmo_interface_get_class(iface, "myclass");
        method = irmo_interface_get_method(iface, "mymethod");

//        assert(irmo_interface_num_classes(iface) == 1);
//        assert(irmo_interface_num_methods(iface) == 1);
        assert(irmo_class_num_variables(klass) == 1);
        assert(irmo_method_num_arguments(method) == 1);

        irmo_interface_unref(iface);
}

int main(int argc, char *argv[])
{
        test_build_interface();
        test_duplicates();
        test_counts();

        return 0;
}

