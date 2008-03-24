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
 * Inside an IrmoWorld, objects (see @ref IrmoObject)
 * of classes defined within that interface can be created. These 
 * objects can be used to hold data of various types.
 *
 * Callback functions can be set to be called automatically when particular
 * objects change within the World.
 *
 * @addtogroup world
 * \{
 */

/*!
 * Create a new World from an Interface. The classes defined in the
 * interface can then be instantiated as objects within the world.
 *
 * @param iface	The interface to use.
 * @return	The new world.
 */

IrmoWorld *irmo_world_new(IrmoInterface *iface);

/*!
 * Find an object by its numerical ID.
 *
 * All objects within an @ref IrmoWorld have a unique number assigned to
 * them (see @ref IrmoObjectID). This function searches for an object 
 * by its identifier.
 *
 * @param world         The @ref IrmoWorld to search in.
 * @param id		The ID of the @ref IrmoObject to retrieve.
 * @return		The @ref IrmoObject or NULL if the object is
 *                      not found.
 */

IrmoObject *irmo_world_get_object_for_id(IrmoWorld *world,
					 IrmoObjectID id);

/*!
 * Iterate over objects in an @ref IrmoWorld.
 *
 * This function creates an @ref IrmoIterator to iterate over all
 * objecs in a world.
 *
 * @param world	        The world to iterate over.
 * @param classname     The name of the class of objects to iterate over.
 *                      If you want to iterate over ALL objects, pass
 *                      NULL for this value.
 * @return              Pointer to an @ref IrmoIterator object to iterate
 *                      over objects in the world.
 */

IrmoIterator *irmo_world_iterate_objects(IrmoWorld *world, char *classname);

/*!
 * Get the interface for an @ref IrmoWorld.
 *
 * Returns the @ref IrmoInterface object specifying the interface for a
 * given world.
 *
 * @param world         The world to query.
 * @return		The @ref IrmoInterface used by the world.
 */

IrmoInterface *irmo_world_get_interface(IrmoWorld *world);

/*!
 * Add a reference to a @ref IrmoWorld.
 *
 * IrmoWorlds implement reference counting. Each time a reference is
 * added to a world, @ref irmo_world_ref should be called to increase
 * the reference count. When a reference is removed, 
 * @ref irmo_world_unref should be called. The count starts at 1. 
 * When the count reaches 0, the world is destroyed.
 *
 * @param world         The world to reference.
 */

void irmo_world_ref(IrmoWorld *world);

/*!
 * Remove a reference to a World.
 *
 * See @ref irmo_world_ref.
 *
 * @param world	The world to unreference.
 */

void irmo_world_unref(IrmoWorld *world);

/*!
 * Watch for creation of new objects in an @ref IrmoWorld. Every time
 * objects of a particular class are created, a callback function will
 * be called.
 *
 * @param world         @ref IrmoWorld to watch.
 * @param classname	Name of the object class to watch. Specify NULL
 *                      to watch for creation of all objects.
 * @param func		The function to call when new objects are
 * 			created.
 * @param user_data	Some extra data to pass to the callback function.
 *
 *
 * @return an @ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_world_watch_new(IrmoWorld *world, 
				   char *classname,
				   IrmoObjCallback func, 
				   void *user_data);

/*!
 * Watch for modification of objects of a particular class.
 * 
 * Whenever objects of a particular class are modified, a callback
 * function will be called. The watch can be set to be called when
 * a particular variable is changed, or when any variable is
 * changed.
 *
 * @param world         The @ref IrmoWorld to watch in.
 * @param classname	The name of the class to watch. Specify NULL to
 *                      watch for changes to objects of all classes.
 * @param variable	The name of the variable to watch. Specify NULL to
 *                      watch for changes to all variables.
 * 			pass NULL for this value.
 * @param func		A function to call.
 * @param user_data	Some extra data to pass to the callback function.
 *
 * @return              An @ref IrmoCallback object representing the
 *                      watch.
 */

IrmoCallback *irmo_world_watch_class(IrmoWorld *world,
				     char *classname, char *variable,
				     IrmoVarCallback func, 
				     void *user_data);

/*!
 * Watch for object destruction.
 *
 * Whenever any object of a particular class is about to be destroyed,
 * a callback function will first be invoked.
 *
 * @param world	        The world to watch in.
 * @param classname	The name of the class of object to watch. Specify
 *                      NULL to watch for destruction of objects of all
 * 		        classes.
 * @param func		The function to call.
 * @param user_data	Some extra data to pass to the callback function.
 *
 * @return              An @ref IrmoCallback object representing the
 *                      watch.
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

