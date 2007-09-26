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
// IrmoInterface
//

IrmoClass *irmo_interface_get_class(IrmoInterface *iface, 
					 char *class_name)
{
	irmo_return_val_if_fail(iface != NULL, NULL);
	irmo_return_val_if_fail(class_name != NULL, NULL);

	return irmo_hash_table_lookup(iface->class_hash, class_name);
}

IrmoMethod *irmo_interface_get_method(IrmoInterface *iface, 
					   char *method_name)
{
	irmo_return_val_if_fail(iface != NULL, NULL);
	irmo_return_val_if_fail(method_name != NULL, NULL);

	return irmo_hash_table_lookup(iface->method_hash, method_name);
}

void irmo_interface_foreach_class(IrmoInterface *iface, 
				       IrmoClassCallback func, 
				       void *user_data)
{
	int i;

	irmo_return_if_fail(iface != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<iface->nclasses; ++i) 
		func(iface->classes[i], user_data);
}

void irmo_interface_foreach_method(IrmoInterface *iface, 
					IrmoMethodCallback func, 
					void *user_data)
{
	int i;

	irmo_return_if_fail(iface != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<iface->nmethods; ++i) 
		func(iface->methods[i], user_data);
}


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


//
// IrmoClassVar
//

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

void irmo_class_var_ref(IrmoClassVar *var)
{
	irmo_return_if_fail(var != NULL);

	irmo_interface_ref(var->parent->parent);
}

void irmo_class_var_unref(IrmoClassVar *var)
{
	irmo_return_if_fail(var != NULL);

	irmo_interface_unref(var->parent->parent);
}

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

//
// IrmoMethodArg
//

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

	irmo_interface_ref(arg->parent->parent);
}

void irmo_method_arg_unref(IrmoMethodArg *arg)
{
	irmo_return_if_fail(arg != NULL);

	irmo_interface_unref(arg->parent->parent);
}

