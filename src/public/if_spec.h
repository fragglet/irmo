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

#include <glib.h>

/*!
 * \addtogroup if_spec
 * \{
 */

/*!
 * \brief Variable types
 */

typedef enum {
	IRMO_TYPE_UNKNOWN,
	IRMO_TYPE_INT8,
	IRMO_TYPE_INT16,
	IRMO_TYPE_INT32,
	IRMO_TYPE_STRING,
	IRMO_NUM_TYPES,
} IrmoVarType;

/*!
 * \brief An object representing a class in an interface
 */

typedef struct _IrmoClass IrmoClass;

/*!
 * \brief An object representing a variable in a class
 */

typedef struct _IrmoClassVar IrmoClassVar;

/*!
 * \brief An object representing a method in an interface
 */

typedef struct _IrmoMethod IrmoMethod;

/*!
 * \brief An object representing an argument to a method
 */ 

typedef struct _IrmoMethodArg IrmoMethodArg;

/*!
 * \brief An IrmoInterfaceSpec object
 * \ingroup if_spec
 */

typedef struct _IrmoInterfaceSpec IrmoInterfaceSpec;

typedef void (*IrmoClassCallback)(IrmoClass *klass, gpointer user_data);
typedef void (*IrmoClassVarCallback)(IrmoClassVar *var, gpointer user_data);
typedef void (*IrmoMethodCallback)(IrmoMethod *method, gpointer user_data);
typedef void (*IrmoMethodArgCallback)(IrmoMethodArg *arg, gpointer user_data);

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
					 gchar *class_name);

/*!
 * \brief Iterate over all classes in a specification
 * 
 * \param spec		The specification object
 * \param func		A user function to call for each class
 * \param user_data	Extra data to pass to the function
 */

void irmo_interface_spec_foreach_class(IrmoInterfaceSpec *spec, 
				       IrmoClassCallback func, 
				       gpointer user_data);
/*!
 * \brief Get the \ref IrmoMethod object representing a particular method
 *
 * \param spec		The interface specification
 * \param method_name 	The name of the method
 *
 * \return 		A pointer to the object or NULL if it does not exist
 */

IrmoMethod *irmo_interface_spec_get_method(IrmoInterfaceSpec *spec, 
					   gchar *method_name);

/*!
 * \brief Iterate over all methods in a specification
 * 
 * \param spec		The specification object
 * \param func		A user function to call for each method
 * \param user_data	Extra data to pass to the function
 */

void irmo_interface_spec_foreach_method(IrmoInterfaceSpec *spec, 
					IrmoMethodCallback func, 
					gpointer user_data);

/*! 
 * \brief Get the name of a \ref IrmoClass object
 */

gchar *irmo_class_get_name(IrmoClass *klass);

/*!
 * \brief Get the number of member variables in an \ref IrmoClass
 */

gint irmo_class_num_variables(IrmoClass *klass);

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

IrmoClassVar *irmo_class_get_variable(IrmoClass *klass, gchar *var_name);

/*!
 * \brief Add a reference to a \ref IrmoClass object
 */

void irmo_class_ref(IrmoClass *klass);

/*!
 * \brief Remove a reference to a \ref IrmoClass object
 */

void irmo_class_unref(IrmoClass *klass);

/*!
 * \brief Iterate over all variables in a class
 * 
 * \param klass		The class object
 * \param func		A user function to call for each variable
 * \param user_data	Extra data to pass to the function
 */

void irmo_class_foreach_variable(IrmoClass *klass, 
				 IrmoClassVarCallback func, 
				 gpointer user_data);

/*!
 * \brief Get the name of a \ref IrmoClassVar object
 */

gchar *irmo_class_var_get_name(IrmoClassVar *var);

/*!
 * \brief Get the type of a \ref IrmoClassVar object 
 */

IrmoVarType irmo_class_var_get_type(IrmoClassVar *var);

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

gchar *irmo_method_get_name(IrmoMethod *method);

/*!
 * \brief Get the number of arguments to the method
 */

gint irmo_method_num_arguments(IrmoMethod *method);

/*!
 * \brief Get the \ref IrmoMethodArg object representing a particular class
 *        variable
 *
 * \param method	The \ref IrmoMethod object representing the method
 * \param arg_name	The argument name
 *
 * \return 		A pointer to the object or NULL if it does not exist
 */

IrmoMethodArg *irmo_method_get_argument(IrmoMethod *method, gchar *arg_name);

/*!
 * \brief Iterate over all arguments to a method
 * 
 * \param method	The class object
 * \param func		A user function to call for each variable
 * \param user_data	Extra data to pass to the function
 */

void irmo_method_foreach_argument(IrmoMethod *method,
				  IrmoMethodArgCallback func, 
				  gpointer user_data);

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

gchar *irmo_method_arg_get_name(IrmoMethodArg *arg);

/*! 
 * \brief Get the type of a method argument
 */

IrmoVarType irmo_method_arg_get_type(IrmoMethodArg *arg);

/*!
 * \brief Add a reference to an \ref IrmoMethodArg object
 */

void irmo_method_arg_ref(IrmoMethodArg *arg);

/*!
 * \brief Remove a reference to an \ref IrmoMethodArg object
 */

void irmo_method_arg_unref(IrmoMethodArg *arg);

//! \}

#endif /* #ifndef IFSPEC_H */

// $Log$
// Revision 1.9  2003/09/02 20:33:55  fraggle
// Subclassing in interfaces
//
// Revision 1.8  2003/08/31 22:24:26  fraggle
// Add missing prototypes to headers
//
// Revision 1.7  2003/08/29 17:25:24  fraggle
// IRMO_NUM_TYPES
//
// Revision 1.6  2003/08/29 16:46:33  fraggle
// Use "foreach" instead of "each" for consistency
//
// Revision 1.5  2003/08/29 16:28:19  fraggle
// Iterators for reflection API. Rename IrmoMethodCallback to IrmoInvokeCallback
// to avoid name conflict.
//
// Revision 1.4  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.3  2003/08/21 14:32:29  fraggle
// Initial reflection API
//
// Revision 1.2  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.4  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.3  2003/03/07 12:17:22  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.2  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.1  2003/02/23 00:00:06  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.5  2002/11/17 22:01:58  sdh300
// InterfaceSpec documentation
//
// Revision 1.4  2002/11/13 13:56:24  sdh300
// add some documentation
//
// Revision 1.3  2002/10/29 15:58:26  sdh300
// add index entry for ClassSpec, use "index" instead of n for index
// member name
//
// Revision 1.2  2002/10/21 15:32:34  sdh300
// variable value setting
//
// Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
// initial import
//
