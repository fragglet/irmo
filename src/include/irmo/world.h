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

//
// Irmo World
//

#ifndef IRMO_WORLD_H
#define IRMO_WORLD_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * An IrmoWorld defines a data structure which implements an Interface.
 * Inside an IrmoWorld, objects (see \ref IrmoObject)
 * of classes defined within that interface can be created. These 
 * objects can be used to hold data of various types.
 *
 * Callback functions can be set to be called automatically when particular
 * objects change within the World.
 *
 * \addtogroup world
 * \{
 */

/*!
 * \brief Create a new World
 *
 * Create a new World from an Interface. The classes
 * defined in the interface can then be instantiated as objects
 * within the world.
 *
 * \param iface	The Interface to use.
 * \return	The new world.
 */

IrmoWorld *irmo_world_new(IrmoInterface *iface);

/*!
 * \brief Find an object by its ID
 *
 * All objects within an IrmoWorld have a unique number assigned to
 * them (see \ref IrmoObjectID). This function searches for an object 
 * by its identifier.
 *
 * \param world	The World to search in
 * \param id		The object number to search for
 * \return		The IrmoObject or NULL if the object is not found.
 */

IrmoObject *irmo_world_get_object_for_id(IrmoWorld *world,
					 IrmoObjectID id);

/*!
 * \brief Iterate over objects in a World
 *
 * This function allows you to iterate over objects in a World.
 * For each object found, a function is called with a pointer to the
 * object. The function can be used to iterate over ALL objects or
 * just ones of a particular class.
 *
 * \param world	The world to iterate over
 * \param classname	The name of the class of objects to iterate over.
 * 			If you want to iterate over ALL objects, pass
 * 			NULL for this value.
 * \param func		The callback function to call.
 * \param user_data	Extra data to pass to the functions when they 
 * 			are called.
 */

void irmo_world_foreach_object(IrmoWorld *world, char *classname,
			       IrmoObjCallback func, void *user_data);

/*!
 * \brief	Get the interface for a World
 *
 * Returns the \ref IrmoInterface object specifying the interface for a
 * given world.
 *
 * \param world	The world to query
 * \return		The interface object
 */

IrmoInterface *irmo_world_get_interface(IrmoWorld *world);

/*!
 * \brief	Add a reference to a World.
 *
 * Worlds implement reference counting. Each time you store a 
 * reference to a world, call irmo_world_ref to increase the reference
 * count. When you remove a reference, call \ref irmo_world_unref. The
 * count starts at 1. When the count reaches 0, the world is 
 * destroyed.
 *
 * \param world	The world to reference
 */

void irmo_world_ref(IrmoWorld *world);

/*!
 * \brief	Remove a reference to a World.
 *
 * See \ref irmo_world_ref.
 *
 * \param world	The world to unreference.
 */

void irmo_world_unref(IrmoWorld *world);

/*!
 * \brief Watch for creation of new objects.
 *
 * Watch for creation of new objects in a World. Every time objects
 * of a particular class are created, a callback function will be
 * called.
 *
 * \param world	World to watch in.
 * \param classname	The object class to watch. Specify NULL to watch
 *                      for creation of all objects.
 * \param func		The function to call when new objects are
 * 			created.
 * \param user_data	Some extra data to pass to the callback function.
 *
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_world_watch_new(IrmoWorld *world, 
				   char *classname,
				   IrmoObjCallback func, 
				   void *user_data);

/*!
 * \brief Watch for modification of objects of a particular class.
 * 
 * Whenever objects of a particular class are modified, a callback
 * function will be called. The watch can be set to be called when
 * a particular variable is changed, or when any variable is
 * changed.
 *
 * \param world	The world to watch in.
 * \param classname	The class to watch. Specify NULL to watch for
 *                      changes to objects of all classes.
 * \param variable	The variable name to watch. Specify NULL to
 *                      watch for changes to all variables.
 * 			pass NULL for this value.
 * \param func		A function to call.
 * \param user_data	Some extra data to pass to the callback function.
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_world_watch_class(IrmoWorld *world,
				     char *classname, char *variable,
				     IrmoVarCallback func, 
				     void *user_data);

/*!
 * \brief Watch for object destruction.
 *
 * Whenever any object of a particular class is about to be destroyed,
 * a callback function will first be called.
 *
 * \param world	The world to watch in.
 * \param classname	The name of the class of object to watch. Specify
 *                      NULL to watch for destruction of objects of all
 * 		        classes.
 * \param func		The function to call.
 * \param user_data	Some extra data to pass to the callback function.
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_world_watch_destroy(IrmoWorld *world, 
				       char *classname,
				       IrmoObjCallback func, 
				       void *user_data);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_WORLD_H */

