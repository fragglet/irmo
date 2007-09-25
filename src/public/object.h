// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
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
//---------------------------------------------------------------------

//
// Irmo Objects
//

#ifndef IRMO_OBJECT_H
#define IRMO_OBJECT_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * An \ref IrmoObject exists within a World (see \ref IrmoWorld). They 
 * have a class which defines member variables which hold data. The
 * classes are defined by the \ref IrmoInterfaceSpec which the World
 * is using. The object member variables can hold several different 
 * types of data (int, string) and the values of the variables can
 * be changed and retrieved. Callback functions can be set to watch
 * when the values of these variables change.
 *
 * \addtogroup object
 * \{
 */

/*!
 * \brief Create a new object of a particular class
 *
 * \param world World to create the object within
 * \param type_name The name of the class of object to create
 * \return 	   The created object or NULL for failure
 */

IrmoObject *irmo_object_new(IrmoWorld *world, char *type_name);

/*!
 * \brief Destroy an object
 *
 * \param object  The object to destroy
 */

void irmo_object_destroy(IrmoObject *object);

/*!
 * \brief Get numerical object identifier
 *
 * Each object within a World has a unique number assigned to it. 
 * This function allows you to retrieve the number assigned to a particular
 * object.
 *
 * \param object  The object to query
 * \return 	  Object ID
 */

IrmoObjectID irmo_object_get_id(IrmoObject *object);

/*!
 * \brief Get the class of an object
 *
 * All objects have a class, which is one of the classes defined in the
 * IrmoInterfaceSpec for the World the object exists in. This retrieves
 * the name of the class for a particular object.
 *
 * \param object The object to query.
 * \return 	 A string with the classname
 *
 * \sa irmo_object_get_class_obj
 */

char *irmo_object_get_class(IrmoObject *object);

/*!
 * \brief Get the class object representing the class of an object
 *
 * This is similar to \ref irmo_object_get_class, except that this
 * function returns the class object and not just the name of the
 * class.
 *
 * \param object  The object to query
 * \return        The class of the object
 */

IrmoClass *irmo_object_get_class_obj(IrmoObject *object);

/*!
 * \brief Set the value of an object variable (int type)
 *
 * Set the value of a member variable. This is for use on variables of
 * int type. To set string values, use \ref irmo_object_set_string
 *
 * \param object   The object to change
 * \param variable The name of the variable to change
 * \param value    The new value for the variable
 */

void irmo_object_set_int(IrmoObject *object, char *variable, 
			 unsigned int value);

/*!
 * \brief Set the value of an object variable (string type)
 *
 * Set the value of a member variable. This is for use on variables of
 * string type. To set integer values, use \ref irmo_object_set_int
 *
 * \param object   The object to change
 * \param variable The name of the variable to change
 * \param value    The new value for the variable
 */

void irmo_object_set_string(IrmoObject *object, char *variable, char *value);

/*!
 * \brief Get the value of an object variable(int type)
 *
 * Retrieve the value of an object's member variable. This function is
 * for variables of integer type. To get string values, use 
 * \ref irmo_object_get_string
 *
 * \param object   The object to query
 * \param variable The name of the member variable
 * \return         The value of the member variable
 */

unsigned int irmo_object_get_int(IrmoObject *object, char *variable);

/*!
 * \brief Get the value of an object variable(string type)
 *
 * Retrieve the value of an object's member variable. This function is
 * for variables of string type. To get integer values, use 
 * \ref irmo_object_get_int
 *
 * The returned string should not be modified; to set the value of a 
 * member variable use \ref irmo_object_set_string
 *
 * \param object   The object to query
 * \param variable The name of the member variable
 * \return         The value of the member variable
 */

char *irmo_object_get_string(IrmoObject *object, char *variable);

/*!
 * \brief Get the world an object belongs to
 *
 * Returns a reference to the \ref IrmoWorld world object
 * the object belongs to.
 *
 * \param object    The object to query
 * \return	    The world object the object belongs to
 */

IrmoWorld *irmo_object_get_world(IrmoObject *object);

/*!
 * \brief Find if an object is an instance of a particular class
 *
 * Returns TRUE if the object is an instance of the class specified 
 * or any of its subclasses.
 *
 * \param object	The object
 * \param classname	The name of the class
 */

unsigned int irmo_object_is_a(IrmoObject *object, char *classname);

/*!
 * \brief Find if an object is an instance of a particular class
 *
 * This is the same as \ref irmo_object_is_a, except takes a 
 * reference to an \ref IrmoClass object instead of a class name.
 *
 */

unsigned int irmo_object_is_a2(IrmoObject *object, IrmoClass *klass);

/*!
 * \brief Watch for modification of an object
 *
 * Whenever a particular object is modified, a function will be called.
 * The function can be set to be called only when a particular variable
 * is modified, or when any variable in the object is modified.
 *
 * \param object	The object to watch
 * \param variable	The name of the variable to watch. To make the 
 * 			function call whenever any variable in the object
 * 			is modified, pass NULL for this value.
 * \param func		The function to call.
 * \param user_data	Extra data to pass to the function when it is called.
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_object_watch(IrmoObject *object, char *variable,
				IrmoVarCallback func, void *user_data);

/*!
 * \brief Watch for object destruction
 *
 * Before a particular object is about to be destroyed, a function will 
 * be called.
 *
 * \param object	The object to watch
 * \param func		Callback function to call
 * \param user_data	Extra data to pass to the function when called.
 * \sa irmo_object_unwatch_destroy
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_object_watch_destroy(IrmoObject *object,
					IrmoObjCallback func, 
					void *user_data);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_OBJECT_H */

