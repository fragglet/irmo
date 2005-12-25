// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#include "sysheaders.h"

#include "if_spec.h"

//
// IrmoInterfaceSpec
//

IrmoClass *irmo_interface_spec_get_class(IrmoInterfaceSpec *spec, 
					 char *class_name)
{
	irmo_return_val_if_fail(spec != NULL, NULL);
	irmo_return_val_if_fail(class_name != NULL, NULL);

	return irmo_hash_table_lookup(spec->class_hash, class_name);
}

IrmoMethod *irmo_interface_spec_get_method(IrmoInterfaceSpec *spec, 
					   char *method_name)
{
	irmo_return_val_if_fail(spec != NULL, NULL);
	irmo_return_val_if_fail(method_name != NULL, NULL);

	return irmo_hash_table_lookup(spec->method_hash, method_name);
}

void irmo_interface_spec_foreach_class(IrmoInterfaceSpec *spec, 
				       IrmoClassCallback func, 
				       void *user_data)
{
	int i;

	irmo_return_if_fail(spec != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<spec->nclasses; ++i) 
		func(spec->classes[i], user_data);
}

void irmo_interface_spec_foreach_method(IrmoInterfaceSpec *spec, 
					IrmoMethodCallback func, 
					void *user_data)
{
	int i;

	irmo_return_if_fail(spec != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<spec->nmethods; ++i) 
		func(spec->methods[i], user_data);
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

	irmo_interface_spec_ref(klass->parent);
}

void irmo_class_unref(IrmoClass *klass)
{
	irmo_return_if_fail(klass != NULL);

	irmo_interface_spec_unref(klass->parent);
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

	irmo_interface_spec_ref(var->parent->parent);
}

void irmo_class_var_unref(IrmoClassVar *var)
{
	irmo_return_if_fail(var != NULL);

	irmo_interface_spec_unref(var->parent->parent);
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

	irmo_interface_spec_ref(method->parent);
}

void irmo_method_unref(IrmoMethod *method)
{
	irmo_return_if_fail(method != NULL);

	irmo_interface_spec_unref(method->parent);
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

	irmo_interface_spec_ref(arg->parent->parent);
}

void irmo_method_arg_unref(IrmoMethodArg *arg)
{
	irmo_return_if_fail(arg != NULL);

	irmo_interface_spec_unref(arg->parent->parent);
}

// $Log$
// Revision 1.13  2005/12/25 00:38:18  fraggle
// Use internal macros instead of glib ones for assertation checks
//
// Revision 1.12  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.11  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.10  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.9  2003/09/12 11:38:19  fraggle
// Make it possible to get method arguments by argument number
//
// Revision 1.8  2003/09/12 11:30:25  fraggle
// Rename IrmoVarType to IrmoValueType to be orthogonal to IrmoValue
//
// Revision 1.7  2003/09/03 15:46:40  fraggle
// In reflection functions, do not include variables inherited from
// parent classes.
//
// Revision 1.6  2003/09/02 20:33:55  fraggle
// Subclassing in interfaces
//
// Revision 1.5  2003/08/31 18:32:10  fraggle
// refcounting functions for the InterfaceSpec internals
//
// Revision 1.4  2003/08/29 16:46:33  fraggle
// Use "foreach" instead of "each" for consistency
//
// Revision 1.3  2003/08/29 16:28:19  fraggle
// Iterators for reflection API. Rename IrmoMethodCallback to IrmoInvokeCallback
// to avoid name conflict.
//
// Revision 1.2  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.1  2003/08/21 14:32:29  fraggle
// Initial reflection API
//
//
