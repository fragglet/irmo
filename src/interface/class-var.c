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
// IrmoClassVar
//

IrmoClassVar *irmo_class_new_variable(IrmoClass *klass,
                                      char *var_name,
                                      IrmoValueType var_type)
{
        IrmoClassVar *class_var;

        irmo_return_val_if_fail(klass != NULL, NULL);
        irmo_return_val_if_fail(var_name != NULL, NULL);
        irmo_return_val_if_fail(var_type != IRMO_TYPE_UNKNOWN
                             && var_type != IRMO_NUM_TYPES, NULL);

        if (klass->nvariables >= MAX_VARIABLES) {
                irmo_error_report("irmo_class_new_variable", 
                                  "Maximum of %i variables per class",
                                  MAX_VARIABLES);
                return NULL;
        }

        if (irmo_hash_table_lookup(klass->variable_hash, var_name) != NULL) {
                irmo_error_report("irmo_class_new_variable", 
                                  "Variable named '%s' already declared "
                                  "in class '%s'",
                                  var_name, klass->name);
                return NULL;
        }

        // Allocate a new class variable.

        class_var = irmo_new0(IrmoClassVar, 1);

        class_var->name = strdup(var_name);
        class_var->type = var_type;
        class_var->index = klass->nvariables;
        class_var->klass = klass;

        // Add to the class.

        klass->variables = irmo_renew(IrmoClassVar *, klass->variables,
                                      klass->nvariables + 1);
        klass->variables[klass->nvariables] = class_var;
        ++klass->nvariables;

        irmo_hash_table_insert(klass->variable_hash, 
                               class_var->name, class_var);

        return class_var;
}

char *irmo_class_var_get_name(IrmoClassVar *var)
{
	irmo_return_val_if_fail(var != NULL, NULL);

	return var->name;
}

IrmoValueType irmo_class_var_get_type(IrmoClassVar *var)
{
	irmo_return_val_if_fail(var != NULL, IRMO_TYPE_UNKNOWN);

	return var->type;
}

void irmo_class_var_bind(IrmoClassVar *var, char *member_name)
{
        IrmoStructMember *member;

        irmo_return_if_fail(var != NULL);
        irmo_return_if_fail(member_name != NULL);
        irmo_return_if_fail(var->klass->structure != NULL);

        // Find the member with the given name

        member = irmo_struct_get_member(var->klass->structure, member_name);

        irmo_return_if_fail(member != NULL);

        var->member = member;
}

void irmo_class_var_ref(IrmoClassVar *var)
{
	irmo_return_if_fail(var != NULL);

	irmo_interface_ref(var->klass->iface);
}

void irmo_class_var_unref(IrmoClassVar *var)
{
	irmo_return_if_fail(var != NULL);

	irmo_interface_unref(var->klass->iface);
}

uint32_t irmo_class_var_hash(IrmoClassVar *class_var)
{
        return class_var->type
             ^ irmo_string_hash(class_var->name);
}

void _irmo_class_var_free(IrmoClassVar *var)
{
	free(var->name);
	free(var);
}

