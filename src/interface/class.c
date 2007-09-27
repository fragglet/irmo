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

#include "interface.h"

//
// IrmoClass
//

static void copy_parent_variables(IrmoClass *klass)
{
        IrmoClass *parent;
        IrmoClassVar *class_var;
        int i;

        parent = klass->parent_class;

        // Copy all the parent's variables into the new class.
 
        for (i=0; i<parent->nvariables; ++i) {
                class_var = parent->variables[i];

                irmo_class_new_variable(klass, class_var->name, 
                                        class_var->type);
        }
}

IrmoClass *irmo_interface_new_class(IrmoInterface *iface,
                                    char *class_name,
                                    IrmoClass *parent_class)
{
        IrmoClass *klass;

        irmo_return_val_if_fail(iface != NULL, NULL);
        irmo_return_val_if_fail(class_name != NULL, NULL);

        if (iface->nclasses >= MAX_CLASSES) {
                irmo_error_report("irmo_interface_new_class",
                                  "Maximum of %i classes per interface.",
                                  MAX_CLASSES);
                return NULL;
        }

        // Is this already defined?

        if (irmo_hash_table_lookup(iface->class_hash, class_name) != NULL) {
                irmo_error_report("irmo_interface_new_class",
                                  "Class named '%s' already declared.",
                                  class_name);
                return NULL;
        }

        // Allocate a new IrmoClass and populate.

        klass = irmo_new0(IrmoClass, 1);
        klass->name = strdup(class_name);
        klass->variables = NULL;
        klass->nvariables = 0;
        klass->variable_hash = irmo_hash_table_new(irmo_string_hash,
                                                   irmo_string_equal);
        klass->iface = iface;
        klass->parent_class = parent_class;
        klass->index = iface->nclasses;

        // If this class has a parent class, copy all the variables 
        // of the parent class.

        if (parent_class != NULL) {
                copy_parent_variables(klass);
        }

        // Add to the IrmoInterface.
      
        iface->classes = irmo_renew(IrmoClass *, iface->classes, 
                                    iface->nclasses + 1);
        iface->classes[iface->nclasses] = klass;
        ++iface->nclasses;

        irmo_hash_table_insert(iface->class_hash, klass->name, klass);

        return klass;
}

char *irmo_class_get_name(IrmoClass *klass)
{
	irmo_return_val_if_fail(klass != NULL, NULL);

	return klass->name;
}

int irmo_class_num_variables(IrmoClass *klass)
{
	irmo_return_val_if_fail(klass != NULL, -1);

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

	irmo_return_if_fail(klass != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<klass->nvariables; ++i) {
		func(klass->variables[i], user_data);
        }
}

IrmoClass *irmo_class_parent_class(IrmoClass *klass)
{
	irmo_return_val_if_fail(klass != NULL, NULL);

	return klass->parent_class;
}

void irmo_class_ref(IrmoClass *klass)
{
	irmo_return_if_fail(klass != NULL);

	irmo_interface_ref(klass->iface);
}

void irmo_class_unref(IrmoClass *klass)
{
	irmo_return_if_fail(klass != NULL);

	irmo_interface_unref(klass->iface);
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

	irmo_hash_table_free(klass->variable_hash);

	for (i=0; i<klass->nvariables; ++i)
		_irmo_class_var_free(klass->variables[i]);

	free(klass->variables);
	free(klass->name);
	free(klass);
}

