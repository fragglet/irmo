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
// Irmo Objects
//

#ifndef IRMO_INTERNAL_OBJECT_H
#define IRMO_INTERNAL_OBJECT_H

#include "public/object.h"

#include "callback.h"
#include "if_spec.h"

// internal stuff:

struct _IrmoObject {

	// world this object is attached to

	IrmoWorld *world;

	// callback data for this object
	
	IrmoCallbackData *callbacks;

	// the class of this object, one of the classes specified
	// in the interface spec for the world this object exists
	// in.
	
	IrmoClass *objclass;

	// numerical id reference for this object in the world
	
	IrmoObjectID id;

	// array of variables for this object
	// the number of variables is specified in objclass
	
	IrmoValue *variables;

	// position in stream from remote server where variable
	// was last changed

	int *variable_time;
};

// internal function create a new object

IrmoObject *irmo_object_internal_new(IrmoWorld *world,
				     IrmoClass *objclass,
				     IrmoObjectID id);

// internal function to destroy an object. control over whether to
// call notify routines (call callbacks, forward info to clients)
// and whether to remove from the world.

void irmo_object_internal_destroy(IrmoObject *object, int notify,
				  int remove);


// internal function to raise notify functions for when an object
// is modified. callback functions are called and the change forwarded
// to connected clients. needs object and index of variable number
// modified.

void irmo_object_set_raise(IrmoObject *object, int variable);


#endif /* #ifndef IRMO_OBJECT_H */

// $Log$
// Revision 1.8  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.7  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.6  2003/11/17 00:32:28  fraggle
// Rename irmo_objid_t to IrmoObjectID for consistency with other types
//
// Revision 1.5  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.4  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.3  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.2  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.19  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.18  2003/05/20 02:06:06  sdh300
// Add out-of-order execution of stream
//
// Revision 1.17  2003/03/07 14:25:40  sdh300
// Fix ordering of IrmoVariable typedef
//
// Revision 1.16  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.15  2003/03/06 20:58:50  sdh300
// Add documentation for IrmoObject data
//
// Revision 1.14  2003/03/06 19:21:26  sdh300
// Split off some of the constructor/destructor/change code into
// seperate functions that can be reused elsewhere
//
// Revision 1.13  2003/02/23 01:01:01  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.12  2003/02/23 00:00:03  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.11  2002/11/17 20:37:15  sdh300
// initial doxygen documentation
//
// Revision 1.10  2002/11/13 15:12:34  sdh300
// object_get_id to get identifier
//
// Revision 1.9  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.8  2002/11/13 13:57:42  sdh300
// object_get_class to get the class of an object
//
// Revision 1.7  2002/11/13 13:56:24  sdh300
// add some documentation
//
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/29 14:48:16  sdh300
// variable value retrieval
//
// Revision 1.4  2002/10/21 15:39:36  sdh300
// setting string values
//
// Revision 1.3  2002/10/21 15:10:17  sdh300
// missing cvs tags
//
