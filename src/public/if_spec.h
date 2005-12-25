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

#ifndef IRMO_IF_SPEC_H
#define IRMO_IF_SPEC_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * An \ref IrmoInterfaceSpec object defines an interface specification 
 * comprising specification for several object classes and methods.
 * The specification can then be used to create an \ref IrmoWorld
 * where the classes can be instantiated as \ref IrmoObject objects.
 *
 * The InterfaceSpec is defined in a seperate file. This has a C-like
 * syntax and is quite simple. The file is loaded with the 
 * \ref irmo_interface_spec_new function.
 *
 * \addtogroup if_spec
 * \{
 */

/*!
 * \brief Create a new IrmoInterfaceSpec object
 *
 * A file is parsed with the interface described in a simple
 * C-style syntax.
 *
 * \param filename	The filename of the specification file
 * \return		A new IrmoInterfaceSpec object or NULL for failure
 */

IrmoInterfaceSpec *irmo_interface_spec_new(char *filename);

/*!
 * \brief Add a reference to an IrmoInterfaceSpec object
 *
 * Reference counting is implemented for IrmoInterfaceSpec objects. Every time 
 * a new reference is kept, call this to increment the reference count.
 * When a reference is removed, call \ref irmo_interface_spec_unref. The 
 * reference count starts at 1. When the reference count reaches 0, the
 * object is destroyed.
 *
 * \param spec		The object to reference
 */

void irmo_interface_spec_ref(IrmoInterfaceSpec *spec);

/*!
 * \brief Remove a reference to an IrmoInterfaceSpec object
 *
 * See \ref irmo_interface_spec_ref
 *
 * \param spec		The object to unreference
 */

void irmo_interface_spec_unref(IrmoInterfaceSpec *spec);

/*!
 * \brief Get the \ref IrmoClass object representing a particular class
 *
 * \param spec		The interface specification
 * \param class_name 	The name of the class
 *
 * \return 		A pointer to the object or NULL if it does not exist
 */

IrmoClass *irmo_interface_spec_get_class(IrmoInterfaceSpec *spec, 
					 char *class_name);

/*!
 * \brief Iterate over all classes in a specification
 * 
 * \param spec		The specification object
 * \param func		A user function to call for each class
 * \param user_data	Extra data to pass to the function
 */

void irmo_interface_spec_foreach_class(IrmoInterfaceSpec *spec, 
				       IrmoClassCallback func, 
				       void *user_data);
/*!
 * \brief Get the \ref IrmoMethod object representing a particular method
 *
 * \param spec		The interface specification
 * \param method_name 	The name of the method
 *
 * \return 		A pointer to the object or NULL if it does not exist
 */

IrmoMethod *irmo_interface_spec_get_method(IrmoInterfaceSpec *spec, 
					   char *method_name);

/*!
 * \brief Iterate over all methods in a specification
 * 
 * \param spec		The specification object
 * \param func		A user function to call for each method
 * \param user_data	Extra data to pass to the function
 */

void irmo_interface_spec_foreach_method(IrmoInterfaceSpec *spec, 
					IrmoMethodCallback func, 
					void *user_data);

/*! 
 * \brief Get the name of a \ref IrmoClass object
 */

char *irmo_class_get_name(IrmoClass *klass);

/*!
 * \brief Get the number of member variables in an \ref IrmoClass
 *
 * This returns only the number of unique variables. Variables 
 * inherited from parent classes are not included.
 */

int irmo_class_num_variables(IrmoClass *klass);

/*!
 * \brief Get the parent class of a class
 *
 * \return The parent class, or NULL if the class is not a subclass
 *
 */

IrmoClass *irmo_class_parent_class(IrmoClass *klass);

/*!
 * \brief Get the \ref IrmoClassVar object representing a particular class
 *        variable
 *
 * \param klass		The \ref IrmoClass object representing the class
 * \param var_name	The variable name
 *
 * \return 		A pointer to the object or NULL if it does not exist
 */

IrmoClassVar *irmo_class_get_variable(IrmoClass *klass, char *var_name);

/*!
 * \brief Add a reference to a \ref IrmoClass object
 */

void irmo_class_ref(IrmoClass *klass);

/*!
 * \brief Remove a reference to a \ref IrmoClass object
 */

void irmo_class_unref(IrmoClass *klass);

/*!
 * \brief Iterate over variables in a class
 *
 * This function iterates over variables in a class. It only iterates
 * over variables unique to that class; variables inherited from the
 * parent class are not included.
 * 
 * \param klass		The class object
 * \param func		A user function to call for each variable
 * \param user_data	Extra data to pass to the function
 */

void irmo_class_foreach_variable(IrmoClass *klass, 
				 IrmoClassVarCallback func, 
				 void *user_data);

/*!
 * \brief Get the name of a \ref IrmoClassVar object
 */

char *irmo_class_var_get_name(IrmoClassVar *var);

/*!
 * \brief Get the type of a \ref IrmoClassVar object 
 */

IrmoValueType irmo_class_var_get_type(IrmoClassVar *var);

/*!
 * \brief Add a reference to an \ref IrmoClassVar object
 */

void irmo_class_var_ref(IrmoClassVar *var);

/*!
 * \brief Remove a reference to an \ref IrmoClassVar object
 */

void irmo_class_var_unref(IrmoClassVar *var);

/*!
 * \brief Get the name of a \ref IrmoMethod
 */

char *irmo_method_get_name(IrmoMethod *method);

/*!
 * \brief Get the number of arguments to the method
 */

int irmo_method_num_arguments(IrmoMethod *method);

/*!
 * \brief Get the \ref IrmoMethodArg object representing a particular 
 *        method argument
 *
 * \param method	The \ref IrmoMethod object representing the method
 * \param arg_name	The argument name
 *
 * \return 		A pointer to the object or NULL if it does not exist
 */

IrmoMethodArg *irmo_method_get_argument(IrmoMethod *method, char *arg_name);

/*!
 * \brief Get the \ref IrmoMethodArg object representing a particular 
 *        method argument by number
 *
 * This is the same as \ref irmo_method_get_argument, but finds the argument
 * by the number of the argument.
 */

IrmoMethodArg *irmo_method_get_argument2(IrmoMethod *method, int arg_number);

/*!
 * \brief Iterate over all arguments to a method
 * 
 * \param method	The class object
 * \param func		A user function to call for each variable
 * \param user_data	Extra data to pass to the function
 */

void irmo_method_foreach_argument(IrmoMethod *method,
				  IrmoMethodArgCallback func, 
				  void *user_data);

/*!
 * \brief Add a reference to an \ref IrmoMethod object
 */

void irmo_method_ref(IrmoMethod *method);

/*!
 * \brief Remove a reference to an \ref IrmoMethod object
 */

void irmo_method_unref(IrmoMethod *method);

/*! 
 * \brief Get the name of a method argument
 */

char *irmo_method_arg_get_name(IrmoMethodArg *arg);

/*! 
 * \brief Get the type of a method argument
 */

IrmoValueType irmo_method_arg_get_type(IrmoMethodArg *arg);

/*!
 * \brief Add a reference to an \ref IrmoMethodArg object
 */

void irmo_method_arg_ref(IrmoMethodArg *arg);

/*!
 * \brief Remove a reference to an \ref IrmoMethodArg object
 */

void irmo_method_arg_unref(IrmoMethodArg *arg);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IFSPEC_H */

