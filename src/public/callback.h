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

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * Various parts of the library allow callback watches to be placed 
 * on particular events. When the event occurs, the callback function
 * will be invoked. These watches are represented by a 
 * \ref IrmoCallback object.
 *
 * \addtogroup callback
 * \{
 */

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

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_CALLBACK_H */

// $Log$
// Revision 1.7  2003/11/21 18:10:18  fraggle
// Fix up doxygen documentation; move section documentation into headers
// from 'sections.doxygen'
//
// Revision 1.6  2003/11/21 17:46:18  fraggle
// Restructure header files: move type definitions into "types.h"; move
// callback prototypes into their appropriate headers instead of
// callback.h; make headers C++-safe
//
// Revision 1.5  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.4  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
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
