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

#ifndef IRMO_INTERNAL_CALLBACK_H
#define IRMO_INTERNAL_CALLBACK_H

typedef struct _IrmoCallbackData IrmoCallbackData;

#include "public/callback.h"

#include "client.h"
#include "if_spec.h"
#include "object.h"

struct _IrmoCallback {
	GSList **list;                 // callback list this belongs to
	gpointer func;
        gpointer user_data;
};

struct _IrmoCallbackData {

	// Class this callback data is being used for watching
	// if this is NULL, it is for the global callback data
	// for a universe, used to watch all objects.

	ClassSpec *objclass;

	// callbacks for watching for creations of new objects
	// this is only for the class callbackdata - redundant
	// in object callbacks

	GSList *new_callbacks;
	
	// callbacks for if any variable is changed

	GSList *class_callbacks;

	// callbacks for if a particular variable is changed
	// this is redundant if objclass == NULL
	
	GSList **variable_callbacks;

	// callbacks called when object is destroyed
	
	GSList *destroy_callbacks;
};

// generalised callback list functions

IrmoCallback *irmo_callbacklist_add(GSList **list, gpointer func, 
				    gpointer user_data);
void irmo_callbacklist_free(GSList *list);

IrmoCallbackData *callbackdata_new(ClassSpec *objclass);
void callbackdata_free(IrmoCallbackData *data);
void callbackdata_raise(IrmoCallbackData *data,
			 IrmoObject *object, gint variable_index);
void callbackdata_raise_destroy(IrmoCallbackData *data, IrmoObject *object);
void callbackdata_raise_new(IrmoCallbackData *data, IrmoObject *object);

#endif /* #ifndef IRMO_INTERNAL_CALLBACK_H */

// $Log$
// Revision 1.3  2003/08/16 16:45:11  fraggle
// Allow watches on all objects regardless of class
//
// Revision 1.2  2003/07/22 02:05:39  fraggle
// Move callbacks to use a more object-oriented API.
//
// Revision 1.1.1.1  2003/06/09 21:33:23  fraggle
// Initial sourceforge import
//
// Revision 1.17  2003/06/09 21:06:50  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.16  2003/03/16 00:44:04  sdh300
// Add irmo_callbacklist_free function
//
// Revision 1.15  2003/03/14 18:31:36  sdh300
// Generalise callback functions to irmo_callbacklist type,
// remove redundant client_callback code
//
// Revision 1.14  2003/03/07 14:31:18  sdh300
// Callback functions for watching new client connects
//
// Revision 1.13  2003/02/23 01:01:00  sdh300
// Remove underscores from internal functions
// This is not much of an issue now the public definitions have been split
// off into seperate files.
//
// Revision 1.12  2003/02/23 00:00:03  sdh300
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
