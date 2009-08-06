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
 * An @ref IrmoObject exists within a World (see @ref IrmoWorld). They 
 * have a class which defines member variables which hold data. The
 * classes are defined by the @ref IrmoInterface which the World
 * is using. The object member variables can hold several different 
 * types of data (int, string) and the values of the variables can
 * be changed and retrieved. Callback functions can be set to watch
 * when the values of these variables change.
 *
 * @addtogroup object
 * @{
 */

/*!
 * Create a new object of a particular class.
 *
 * @param world       @ref IrmoWorld to create the object within
 * @param type_name   The name of the class of object to create.
 * @return 	      The created object or NULL for failure.
 */

IrmoObject *irmo_object_new(IrmoWorld *world, char *type_name);

/*!
 * Destroy an object.
 *
 * @param object      The object to destroy.
 */

void irmo_object_destroy(IrmoObject *object);

/*!
 * Get an object's numerical identifier.
 *
 * Each object within a @ref IrmoWorld has a unique number assigned to it. 
 * This function returns the number assigned to a particular object.
 *
 * @param object  The object to query.
 * @return        The object's ID.
 */

IrmoObjectID irmo_object_get_id(IrmoObject *object);

/*!
 * Get the name of the class of an object.
 *
 * All objects have a class, which is one of the classes defined in the
 * @ref IrmoInterface for the world the object exists in. This retrieves
 * the name of the class for a particular object.
 *
 * @param object  The object to query.
 * @return 	  The name of the class.
 *
 * @sa irmo_object_get_class_obj
 */

char *irmo_object_get_class(IrmoObject *object);

/*!
 * Get the class an object.
 *
 * This is similar to @ref irmo_object_get_class, except that this
 * function returns the @ref IrmoClass object and not just the name of the
 * class.
 *
 * @param object  The object to query.
 * @return        The class of the object.
 */

IrmoClass *irmo_object_get_class_obj(IrmoObject *object);

/*!
 * Set the value of an object's member variable (generic)
 *
 * This is a generic function for setting the value of a variable.
 * In general, @ref irmo_object_set_int and @ref irmo_object_set_string
 * are more pleasant to use.  However, this function is faster as no
 * string lookup is needed to find the variable to change.
 *
 * @param object     The object to change.
 * @param variable   The variable to change.
 * @param value      @ref IrmoValue structure containing the new value to
 *                   set.
 */

void irmo_object_set(IrmoObject *object, IrmoClassVar *variable,
                     IrmoValue *value);

/*!
 * Set the value of an object's member variable (int type).
 *
 * This is for use on variables of int type. To set string values,
 * use @ref irmo_object_set_string.
 *
 * @param object     The object to change.
 * @param variable   The name of the variable to change.
 * @param value      The new value for the variable.
 */

void irmo_object_set_int(IrmoObject *object, char *variable, 
			 unsigned int value);

/*!
 * Set the value of an object's member variable (string type).
 *
 * This is for use on variables of string type. To set integer
 * values, use @ref irmo_object_set_int.
 *
 * @param object     The object to change.
 * @param variable   The name of the variable to change.
 * @param value      The new value for the variable.
 */

void irmo_object_set_string(IrmoObject *object, char *variable, char *value);

/*!
 * Get the value of an object's member variable (generic).
 *
 * This is a generic function for getting the value of a variable.
 * In general, @ref irmo_object_get_int and @ref irmo_object_get_string
 * are more pleasant to use.  However, this function is faster as no
 * string lookup is needed to find the variable to change.
 *
 * @param object     The object to query.
 * @param variable   The variable.
 * @param value      Pointer to an @ref IrmoValue structure in which to store
 *                   the result.
 */

void irmo_object_get(IrmoObject *object, IrmoClassVar *variable, 
                     IrmoValue *value);

/*!
 * Get the value of an object's member variable (int type).
 *
 * This function is for variables of integer type. To get string
 * values, use @ref irmo_object_get_string.
 *
 * @param object     The object to query.
 * @param variable   The name of the member variable.
 * @return           The value of the member variable.
 */

unsigned int irmo_object_get_int(IrmoObject *object, char *variable);

/*!
 * Get the value of an object's member variable(string type)
 *
 * This function is for variables of string type. To get integer values, use 
 * @ref irmo_object_get_int.
 *
 * The returned string should not be modified; to set the value of a 
 * member variable use @ref irmo_object_set_string.
 *
 * @param object   The object to query.
 * @param variable The name of the member variable.
 * @return         The value of the member variable.
 */

char *irmo_object_get_string(IrmoObject *object, char *variable);

/*!
 * Get the @ref IrmoWorld world that an object belongs to.
 *
 * @param object    The object to query.
 * @return	    The @ref IrmoWorld the object belongs to.
 */

IrmoWorld *irmo_object_get_world(IrmoObject *object);

/*!
 * Determine if an object is an instance of a particular class.
 *
 * @param object	The object.
 * @param classname	The name of the class.
 * @return              Non-zero if the object is an instance of the
 *                      specified class or any of its subclasses.
 */

int irmo_object_is_a(IrmoObject *object, char *classname);

/*!
 * Determine if an object is an instance of a particular class.
 *
 * This is the same as @ref irmo_object_is_a, except takes a 
 * reference to an @ref IrmoClass object instead of a class name.
 *
 * @param object        The object.
 * @param klass         The class.
 * @return              Non-zero if the object is an instance of the
 *                      specified class or any of its subclasses.
 */

int irmo_object_is_a2(IrmoObject *object, IrmoClass *klass);

/*!
 * Watch for modification of an object.
 *
 * Whenever a particular object is modified, a function will be called.
 * The function can be set to be called only when a particular variable
 * is modified, or when any variable in the object is modified.
 *
 * @param object	The object to watch.
 * @param variable	The name of the variable to watch. To make the 
 * 			function call whenever any variable in the object
 * 			is modified, pass NULL for this value.
 * @param func		The function to call.
 * @param user_data	Extra data to pass to the function when it is called.
 *
 * @return              An @ref IrmoCallback object representing the watch.
 */

IrmoCallback *irmo_object_watch(IrmoObject *object, char *variable,
				IrmoVarCallback func, void *user_data);

/*!
 * Watch for object destruction.
 *
 * Before a particular object is about to be destroyed, a function will 
 * be called.
 *
 * @param object	The object to watch.
 * @param func		Callback function to call.
 * @param user_data	Extra data to pass to the function when called.
 * @sa irmo_object_unwatch_destroy
 *
 * @return              An @ref IrmoCallback object representing the watch.
 */

IrmoCallback *irmo_object_watch_destroy(IrmoObject *object,
					IrmoObjCallback func, 
					void *user_data);

/*!
 * Attach a user-specified pointer to an object.
 *
 * @param object        The object.
 * @param value         The value to attach.
 */

void irmo_object_set_data(IrmoObject *object, void *value);

/*!
 * Get the value of the user-specified pointer attached to an object.
 *
 * @param object        The object.
 * @return              The user-specified pointer value, or NULL if none
 *                      has been set.
 */

void *irmo_object_get_data(IrmoObject *object);

/*!
 * Bind the specified object to the specified C structure, so that
 * changes made to the member variables of the structure automatically
 * update the variables of the object.
 *
 * If the object belongs to a world updated by a remote server, the
 * reverse will also happen; the variables in the C structure will
 * be updated as changes are received from the server.
 *
 * @param obj           The object to bind.
 * @param cstruct       Pointer to the C structure to which the object
 *                      should be bound.  If this is NULL, the binding
 *                      is removed.
 */

void irmo_object_bind(IrmoObject *obj, void *cstruct);

/*!
 * Update the specified object automatically based on the variables
 * in the C structure that the object is bound to.  The binding
 * must have already been set using @ref irmo_object_bind.
 *
 * This cannot be done if the object belongs to a world updated by a
 * remote server.
 *
 * @param obj           The object to update.
 */

void irmo_object_update(IrmoObject *obj);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_OBJECT_H */

