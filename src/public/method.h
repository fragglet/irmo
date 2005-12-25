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

#ifndef IRMO_METHOD_H
#define IRMO_METHOD_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * Method calls can be defined and invoked on \ref IrmoWorld objects.
 * Callback functions can be set to be called when methods are invoked.
 * The methods and their arguments are declared in the Interface 
 * Specification for the World.
 *
 * \addtogroup method
 * \{
 */

/*!
 * \brief Set a callback function to be invoked when a method is called
 *
 * \param world    The world to set the watch.
 * \param method_name The name of the method to watch
 * \param method      A callback function to call when the method is invoked.
 * \param user_data   User data to be passed to the callback function
 *
 * \return            A \ref IrmoCallback object representing the watch.
 */

IrmoCallback *irmo_world_method_watch(IrmoWorld *world, 
				      char *method_name,
				      IrmoInvokeCallback method, 
				      void *user_data);

/*!
 * \brief Retrieve a method argument
 *
 * Get the value of a string argument to a method from the method callback
 * function.
 *
 * \param data    A \ref IrmoMethodData object containing information about
 *                the method call
 * \param argname The name of the method argument
 * \return        The value of the method argument (constant string)
 */

char *irmo_method_arg_string(IrmoMethodData *data, char *argname);

/*!
 * \brief Retrieve a method argument
 *
 * Get the value of an integer argument to a method from the method callback
 * function.
 *
 * \param data    A \ref IrmoMethodData object containing information about
 *                the method call
 * \param argname The name of the method argument
 * \return        The value of the method argument
 */

unsigned int irmo_method_arg_int(IrmoMethodData *data, char *argname);

/*!
 * \brief Find the client which invoked a method
 *
 * This function returns a reference to the \ref IrmoClient object
 * of the client which invoked the method. If it was invoked by a
 * function in the local program, it returns NULL.
 *
 */

IrmoClient *irmo_method_get_source(IrmoMethodData *data);

/*!
 * \brief Call a method
 *
 * Invoke a method. If the world is a local copy of a world served from
 * a remote machine the call will be forwarded to the remote machine.
 *
 * Following the function arguments are the method arguments, in the order
 * specified in the interface specification.
 * 
 * \param world  The world object on which to invoke the method
 * \param method    The name of the method
 *
 * \sa irmo_world_method_call2
 */

void irmo_world_method_call(IrmoWorld *world, char *method, ...);

/*!
 * \brief Call a method
 *
 * Invoke a method. This is the same as \ref irmo_world_method_call
 * except it takes an array of \ref IrmoValue structures for the 
 * arguments instead of using the C varargs mechanism.
 */

void irmo_world_method_call2(IrmoWorld *world, char *method,
			     IrmoValue *arguments);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_METHOD_H */

