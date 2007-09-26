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

#include "interface.h"

//
// IrmoMethod
//

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

	irmo_interface_ref(method->parent);
}

void irmo_method_unref(IrmoMethod *method)
{
	irmo_return_if_fail(method != NULL);

	irmo_interface_unref(method->parent);
}

