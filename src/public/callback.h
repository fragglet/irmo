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

//
// Callbacks
//

#ifndef IRMO_CALLBACK_H
#define IRMO_CALLBACK_H

#include "universe.h"
#include "object.h"

/*!
 * \addtogroup callback
 * \{
 */

/*!
 * \brief Callback object
 * 
 * When a new callback watch is created, an IrmoCallback object is
 * returned representing the watch. The watch can be unset with
 * \ref irmo_callback_unset
 */

typedef struct _IrmoCallback IrmoCallback;

/*!
 * \brief Unset a callback watch
 *
 * Unsets a watch on a callback object.
 *
 * \param callback   The callback to destroy.
 *
 */

void irmo_callback_unset(IrmoCallback *callback);

//! \}


/*!
 * \addtogroup universe
 * \{
 */

/*!
 * \brief Watch for creation of new objects.
 *
 * Watch for creation of new objects in a Universe. Every time objects
 * of a particular class are created, a callback function will be
 * called.
 *
 * \param universe	Universe to watch in.
 * \param classname	The object class to watch. Specify NULL to watch
 *                      for creation of all objects.
 * \param func		The function to call when new objects are
 * 			created.
 * \param user_data	Some extra data to pass to the callback function.
 *
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_universe_watch_new(IrmoUniverse *universe, 
				      gchar *classname,
				      IrmoObjCallback func, 
				      gpointer user_data);

/*!
 * \brief Watch for modification of objects of a particular class.
 * 
 * Whenever objects of a particular class are modified, a callback
 * function will be called. The watch can be set to be called when
 * a particular variable is changed, or when any variable is
 * changed.
 *
 * \param universe	The universe to watch in.
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

IrmoCallback *irmo_universe_watch_class(IrmoUniverse *universe,
					gchar *classname, gchar *variable,
					IrmoVarCallback func, 
					gpointer user_data);

/*!
 * \brief Watch for object destruction.
 *
 * Whenever any object of a particular class is about to be destroyed,
 * a callback function will first be called.
 *
 * \param universe	The universe to watch in.
 * \param classname	The name of the class of object to watch. Specify
 *                      NULL to watch for destruction of objects of all
 * 		        classes.
 * \param func		The function to call.
 * \param user_data	Some extra data to pass to the callback function.
 *
 * \return an \ref IrmoCallback object representing the watch
 */

IrmoCallback *irmo_universe_watch_destroy(IrmoUniverse *universe, 
					  gchar *classname,
					  IrmoObjCallback func, 
					  gpointer user_data);

//! \}

/*!
 * \addtogroup object
 * \{
 */

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

IrmoCallback *irmo_object_watch(IrmoObject *object, gchar *variable,
				IrmoVarCallback func, gpointer user_data);

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
					gpointer user_data);

//! \}

#endif /* #ifndef IRMO_CALLBACK_H */

// $Log$
// Revision 1.3  2003/08/16 16:45:11  fraggle
// Allow watches on all objects regardless of class
//
// Revision 1.2  2003/07/22 02:05:39  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.3  2003/06/09 21:06:54  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.2  2003/03/07 12:17:21  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.1  2003/02/23 00:00:06  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.11  2002/11/17 20:37:14  sdh300
// initial doxygen documentation
//
// Revision 1.10  2002/11/13 14:14:45  sdh300
// object iterator function
//
// Revision 1.9  2002/11/12 23:04:31  sdh300
// callback removal, and code cleanup/generalisation
//
// Revision 1.8  2002/11/05 22:33:26  sdh300
// more name changes
//
// Revision 1.7  2002/11/05 16:28:10  sdh300
// new object callbacks
//
// Revision 1.6  2002/11/05 15:55:12  sdh300
// object destroy callbacks
//
// Revision 1.5  2002/11/05 15:17:17  sdh300
// more consistent naming for callback types
//
// Revision 1.4  2002/11/05 15:01:06  sdh300
// change callback function names
// initial destroy callback variables
//
// Revision 1.3  2002/10/29 16:28:50  sdh300
// functioning callbacks
//
// Revision 1.2  2002/10/29 16:10:19  sdh300
// add missing cvs tags
//
