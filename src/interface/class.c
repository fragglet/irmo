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

#include "interface.h"

//
// IrmoClass
//

char *irmo_class_get_name(IrmoClass *klass)
{
	irmo_return_val_if_fail(klass != NULL, NULL);

	return klass->name;
}

int irmo_class_num_variables(IrmoClass *klass)
{
	irmo_return_val_if_fail(klass != NULL, -1);

	if (klass->parent_class)
		return klass->nvariables - klass->parent_class->nvariables;
	else
		return klass->nvariables;
}

IrmoClassVar *irmo_class_get_variable(IrmoClass *klass, char *var_name)
{
	irmo_return_val_if_fail(klass != NULL, NULL);
	irmo_return_val_if_fail(var_name != NULL, NULL);

	return irmo_hash_table_lookup(klass->variable_hash, var_name);
}

void irmo_class_foreach_variable(IrmoClass *klass, 
				 IrmoClassVarCallback func, 
				 void *user_data)
{
	int i;
	int start;

	irmo_return_if_fail(klass != NULL);
	irmo_return_if_fail(func != NULL);

	if (klass->parent_class)
		start = klass->parent_class->nvariables;
	else
		start = 0;

	for (i=start; i<klass->nvariables; ++i)
		func(klass->variables[i], user_data);
}

IrmoClass *irmo_class_parent_class(IrmoClass *klass)
{
	irmo_return_val_if_fail(klass != NULL, NULL);

	return klass->parent_class;
}

void irmo_class_ref(IrmoClass *klass)
{
	irmo_return_if_fail(klass != NULL);

	irmo_interface_ref(klass->parent);
}

void irmo_class_unref(IrmoClass *klass)
{
	irmo_return_if_fail(klass != NULL);

	irmo_interface_unref(klass->parent);
}

uint32_t irmo_class_hash(IrmoClass *klass)
{
	uint32_t hash = 0;
	int i;
	
	for (i=0; i<klass->nvariables; ++i) {
		hash = irmo_rotate_int(hash)
                     ^ irmo_class_var_hash(klass->variables[i]);
	}

	hash ^= irmo_string_hash(klass->name);

	if (klass->parent_class) {
		hash = irmo_rotate_int(hash) ^ klass->parent_class->index;
        }

	return hash;
}

void _irmo_class_free(IrmoClass *klass)
{
	int i;
	int start;

	irmo_hash_table_free(klass->variable_hash);

	// find the start of the range of variables to free
	// (dont free variables from parent class)

	if (klass->parent_class)
		start = klass->parent_class->nvariables;
	else
		start = 0;
	
	for (i=start; i<klass->nvariables; ++i)
		_irmo_class_var_free(klass->variables[i]);

	free(klass->variables);
	free(klass->name);
	free(klass);
}

