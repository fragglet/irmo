// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id: universe.h,v 1.4 2003-06-09 21:06:56 sdh300 Exp $
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
// Irmo Universe
//

#ifndef IRMO_UNIVERSE_H
#define IRMO_UNIVERSE_H

#include <glib.h>

/*!
 * \addtogroup universe
 * \{
 */

//! An Irmo Universe.

typedef struct _IrmoUniverse IrmoUniverse;

/*!
 * \brief A numerical object identifier
 *
 * All objects in a \ref IrmoUniverse have a unique number assigned to
 * them. This can be used to refer to objects by their number. You can
 * search for an object by number using the 
 * \ref irmo_universe_get_object_for_id function.
 */

typedef guint irmo_objid_t;

#include "callback.h"
#include "if_spec.h"
#include "object.h"

/*!
 * \brief Create a new Universe
 *
 * Create a new Universe from an Interface Specification. The classes
 * defined in the specification can then be instantiated as objects
 * within the universe.
 *
 * \param spec	The Interface Specification to use.
 * \return	The new universe.
 */

IrmoUniverse *irmo_universe_new(IrmoInterfaceSpec *spec);

/*!
 * \brief Find an object by its ID
 *
 * All objects within an IrmoUniverse have a unique number assigned to
 * them (see \ref irmo_objid_t). This function searches for an object 
 * by its identifier.
 *
 * \param universe	The Universe to search in
 * \param id		The object number to search for
 * \return		The IrmoObject or NULL if the object is not found.
 */

IrmoObject *irmo_universe_get_object_for_id(IrmoUniverse *universe,
					    irmo_objid_t id);

/*!
 * \brief Iterate over objects in a Universe
 *
 * This function allows you to iterate over objects in a Universe.
 * For each object found, a function is called with a pointer to the
 * object. The function can be used to iterate over ALL objects or
 * just ones of a particular class.
 *
 * \param universe	The universe to iterate over
 * \param classname	The name of the class of objects to iterate over.
 * 			If you want to iterate over ALL objects, pass
 * 			NULL for this value.
 * \param func		The callback function to call.
 * \param user_data	Extra data to pass to the functions when they 
 * 			are called.
 */

void irmo_universe_foreach_object(IrmoUniverse *universe, gchar *classname,
				  IrmoObjCallback func, gpointer user_data);

/*!
 * \brief	Add a reference to a Universe.
 *
 * Universes implement reference counting. Each time you store a 
 * reference to a universe, call irmo_universe_ref to increase the reference
 * count. When you remove a reference, call \ref irmo_universe_unref. The
 * count starts at 1. When the count reaches 0, the universe is 
 * destroyed.
 *
 * \param universe	The universe to reference
 */

void irmo_universe_ref(IrmoUniverse *universe);

/*!
 * \brief	Remove a reference to a Universe.
 *
 * See \ref irmo_universe_ref.
 *
 * \param universe	The universe to unreference.
 */

void irmo_universe_unref(IrmoUniverse *universe);

//! \}

#endif /* #ifndef IRMO_UNIVERSE_H */

// $Log: not supported by cvs2svn $
// Revision 1.3  2003/03/07 12:17:22  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.2  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.1  2003/02/23 00:00:07  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.10  2003/02/18 18:39:54  sdh300
// store list of attached servers in universe
//
// Revision 1.9  2002/11/17 20:37:15  sdh300
// initial doxygen documentation
//
// Revision 1.8  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.7  2002/11/13 13:56:23  sdh300
// add some documentation
//
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/21 14:58:07  sdh300
// split off object code to a seperate module
//
// Revision 1.4  2002/10/21 14:48:54  sdh300
// oops, fix build
//
// Revision 1.3  2002/10/21 14:43:27  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
