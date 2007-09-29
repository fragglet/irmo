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

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "algo/algo.h"

#include "interface.h"

//
// IrmoMethod
//

IrmoMethod *irmo_interface_new_method(IrmoInterface *iface,
                                      char *name)
{
        IrmoMethod *method;

        irmo_return_val_if_fail(iface != NULL, NULL);
        irmo_return_val_if_fail(name != NULL, NULL);

        if (iface->nmethods >= MAX_METHODS) {
                irmo_error_report("irmo_interface_new_method",
                                  "Maximum of %i methods per interface.",
                                  MAX_METHODS);
                return NULL;
        }

        // Already declared?

        if (irmo_hash_table_lookup(iface->method_hash, name) != NULL) {
                irmo_error_report("irmo_interface_new_method",
                                  "Method named '%s' already declared.",
                                  name);
                return NULL;
        }

        // Allocate a new IrmoMethod.

        method = irmo_new0(IrmoMethod, 1);
        method->name = strdup(name);
        method->arguments = NULL;
        method->argument_hash = irmo_hash_table_new(irmo_string_hash, 
                                                    irmo_string_equal);
        method->narguments = 0;
        method->iface = iface;
        method->index = iface->nmethods;

        // Add to the array in the interface.

        iface->methods = irmo_renew(IrmoMethod *, iface->methods, 
                                    iface->nmethods + 1);
        iface->methods[iface->nmethods] = method;
        ++iface->nmethods;

        irmo_hash_table_insert(iface->method_hash, method->name, method);

        return method;
}

char *irmo_method_get_name(IrmoMethod *method)
{
	irmo_return_val_if_fail(method != NULL, NULL);

	return method->name;
}

int irmo_method_num_arguments(IrmoMethod *method)
{
	irmo_return_val_if_fail(method != NULL, -1);

	return method->narguments;
}

IrmoMethodArg *irmo_method_get_argument(IrmoMethod *method, char *arg_name)
{
	irmo_return_val_if_fail(method != NULL, NULL);
	irmo_return_val_if_fail(arg_name != NULL, NULL);

	return irmo_hash_table_lookup(method->argument_hash, arg_name);
}

IrmoMethodArg *irmo_method_get_argument2(IrmoMethod *method, int arg_number)
{
	irmo_return_val_if_fail(method != NULL, NULL);
	irmo_return_val_if_fail(arg_number >= 0 && arg_number < method->narguments,
			     NULL);

	return method->arguments[arg_number];
}

void irmo_method_foreach_argument(IrmoMethod *method,
				  IrmoMethodArgCallback func, 
				  void *user_data)
{
	int i;

	irmo_return_if_fail(method != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<method->narguments; ++i)
		func(method->arguments[i], user_data);
}

void irmo_method_ref(IrmoMethod *method)
{
	irmo_return_if_fail(method != NULL);

	irmo_interface_ref(method->iface);
}

void irmo_method_unref(IrmoMethod *method)
{
	irmo_return_if_fail(method != NULL);

	irmo_interface_unref(method->iface);
}

uint32_t irmo_method_hash(IrmoMethod *method)
{
	uint32_t hash = 0;
	int i;

	for (i=0; i<method->narguments; ++i) {
		hash = irmo_rotate_int(hash)
                     ^ irmo_method_arg_hash(method->arguments[i]);
	}

	hash ^= irmo_string_hash(method->name);
	
	return hash;
}

void _irmo_method_free(IrmoMethod *method)
{
	int i;
	
	irmo_hash_table_free(method->argument_hash);

	for (i=0; i<method->narguments; ++i) 
		_irmo_method_arg_free(method->arguments[i]);
	
	free(method->arguments);
	free(method->name);
	free(method);
}
