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
// Irmo World 
//

#ifndef IRMO_INTERNAL_WORLD_H
#define IRMO_INTERNAL_WORLD_H

#include "public/world.h"

#include <glib.h>

#include "callback.h"
#include "client.h"
#include "if_spec.h"
#include "object.h"

// internals:

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoWorld {

	// specification this world implements
	
	IrmoInterfaceSpec *spec;

	// world-global callback objects, 1 per class
	// used for eg. "watch creation of objects of class my_class"
	// 		"watch for when my_class::my_string is changed"
	
	IrmoCallbackData **callbacks;

	// global callbacks for all objects 
	// eg. "watch destruction of all objects"
	//     "watch for when any object is changed"

	IrmoCallbackData *callbacks_all;

	// objects in the world, hashed by their object id
	
	GHashTable *objects;

	// the id of the last object created. objects are created
	// with sequential ids
	
	IrmoObjectID lastid;

	// servers attached to this world who are serving it.
	
	GPtrArray *servers;

	// number of references to this world
	
	int refcount;

	// if true, this is a local copy of a remote world and
	// cannot be changed
	
	gboolean remote;

	// if this is remote, this is the client from which we are
	// getting the world state

	IrmoClient *remote_client;

	// method callbacks

	GSList **method_callbacks;
};

#endif /* #ifndef IRMO_INTERNAL_WORLD_H */

// $Log$
// Revision 1.3  2003/11/17 00:32:28  fraggle
// Rename irmo_objid_t to IrmoObjectID for consistency with other types
//
// Revision 1.2  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.1  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.2  2003/08/16 16:45:11  fraggle
// Allow watches on all objects regardless of class
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.16  2003/06/09 21:06:53  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.15  2003/03/15 02:21:16  sdh300
// Initial method code
//
// Revision 1.14  2003/03/14 16:53:46  sdh300
// Add structure member for source client for remote universes
//
// Revision 1.13  2003/03/06 20:46:46  sdh300
// Add 'remote' flag. Add documentation for IrmoUniverse data.
//
// Revision 1.12  2003/03/06 19:33:51  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.11  2003/02/23 00:00:04  sdh300
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
