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

/*!
 * \addtogroup method
 * \{
 */

/*!
 * \brief Method Callback Data
 *
 * When a callback function attached to a method is invoked, it is passed
 * an object of this type containing data relevant to the method. For
 * example, the object can be used to retrieve the values of method
 * arguments, and the client which invoked the method.
 */

typedef struct _IrmoMethodData IrmoMethodData;

#include "client.h"
#include "object.h"

/*!
 * \brief Callback function for methods
 *
 * When a method invocation is received from a client, a callback function
 * of this type is invoked.
 *
 * \param data      A \ref IrmoMethodData object holding data about the
 *                  invoked method.
 * \param user_data A pointer to user defined piece of memory specified in
 *                  the \ref irmo_world_method_watch call used to set
 *                  the callback.
 */

typedef void (*IrmoInvokeCallback)(IrmoMethodData *data, 
				   void *user_data);

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

#endif /* #ifndef IRMO_METHOD_H */

// $Log$
// Revision 1.6  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.5  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.4  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.3  2003/08/29 16:28:19  fraggle
// Iterators for reflection API. Rename IrmoMethodCallback to IrmoInvokeCallback
// to avoid name conflict.
//
// Revision 1.2  2003/07/22 02:05:40  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:26  fraggle
// Initial sourceforge import
//
// Revision 1.3  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.2  2003/04/25 01:51:53  sdh300
// Add method API to documentation
//
// Revision 1.1  2003/03/15 02:21:19  sdh300
// Initial method code
//
