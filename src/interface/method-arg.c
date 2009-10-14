//
// Copyright (C) 2002-2008 Simon Howard
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
#include "base/alloc.h"
#include "base/assert.h"
#include "base/error.h"

#include "interface.h"

//
// IrmoMethodArg
//

IrmoMethodArg *irmo_method_new_argument(IrmoMethod *method,
                                        char *arg_name,
                                        IrmoValueType arg_type)
{
        IrmoMethodArg *method_arg;

        irmo_return_val_if_fail(method != NULL, NULL);
        irmo_return_val_if_fail(arg_name != NULL, NULL);
        irmo_return_val_if_fail(arg_type != IRMO_TYPE_UNKNOWN
                             && arg_type != IRMO_NUM_TYPES, NULL);

        if (method->narguments >= MAX_VARIABLES) {
                irmo_error_report("irmo_method_new_argument", 
                                  "Maximum of %i variables per class",
                                  MAX_VARIABLES);
                return NULL;
        }

        if (irmo_hash_table_lookup(method->argument_hash, arg_name) != NULL) {
                irmo_error_report("irmo_method_new_argument", 
                                  "Variable named '%s' already declared in "
                                  "class '%s'.",
                                  arg_name, method->name);
                return NULL;
        }

        // Allocate a new class variable.

        method_arg = irmo_new0(IrmoMethodArg, 1);

        method_arg->name = strdup(arg_name);
        method_arg->type = arg_type;
        method_arg->index = method->narguments;

        // Add to the class.

        method->arguments = irmo_renew(IrmoMethodArg *, method->arguments,
                                       method->narguments + 1);
        method->arguments[method->narguments] = method_arg;
        ++method->narguments;

        irmo_alloc_assert(irmo_hash_table_insert(method->argument_hash,
                                                 method_arg->name,
                                                 method_arg));

        return method_arg;
}

char *irmo_method_arg_get_name(IrmoMethodArg *arg)
{
	irmo_return_val_if_fail(arg != NULL, NULL);

	return arg->name;
}

IrmoValueType irmo_method_arg_get_type(IrmoMethodArg *arg)
{
	irmo_return_val_if_fail(arg != NULL, IRMO_TYPE_UNKNOWN);

	return arg->type;
}

void irmo_method_arg_ref(IrmoMethodArg *arg)
{
	irmo_return_if_fail(arg != NULL);

	irmo_interface_ref(arg->method->iface);
}

void irmo_method_arg_unref(IrmoMethodArg *arg)
{
	irmo_return_if_fail(arg != NULL);

	irmo_interface_unref(arg->method->iface);
}

uint32_t irmo_method_arg_hash(IrmoMethodArg *method_arg)
{
        return method_arg->type ^ irmo_string_hash(method_arg->name);
}

void _irmo_method_arg_free(IrmoMethodArg *arg)
{
	free(arg->name);
	free(arg);
}

