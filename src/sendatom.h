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

#ifndef IRMO_SENDATOM_H
#define IRMO_SENDATOM_H

typedef struct _IrmoSendAtom IrmoSendAtom;

typedef enum {
	ATOM_NULL,               // null atom for nullified changes
	ATOM_NEW,                // new object 
	ATOM_CHANGE,             // modified object 
	ATOM_DESTROY,            // object destroyed 
	ATOM_METHOD,             // method call
	ATOM_SENDWINDOW,         // set maximum sendwindow size
} IrmoSendAtomType;

#include <sys/time.h>

#include "client.h"
#include "if_spec.h"
#include "method.h"
#include "object.h"

// queue object

struct _IrmoSendAtom {
	struct timeval sendtime;        // time this atom was last sent
	gboolean resent;                // this atom was resent
	int len;			// length in packet
	IrmoSendAtomType type;
	
	union {
		struct {
			irmo_objid_t id;
			guint classnum;
		} newobj;
		struct {
			gboolean executed;           // atom has been executed
			irmo_objid_t id;
			IrmoObject *object;

			// count of number of changed variables in this atom
			
			int nchanged;
			
			// array saying which have changed
			
			gboolean *changed;

			// class of the object being changed. this is only
			// used for the receive window.

			IrmoClass *objclass;

			// array of the new values. this is only used for the
			// receive window. for the send window this is NULL
			
			IrmoValue *newvalues;
		} change;
		struct {
			irmo_objid_t id;
		} destroy;
		struct {
			int max;
		} sendwindow;
		IrmoMethodData method;
	} data;
};

void irmo_sendatom_free(IrmoSendAtom *atom);

// create a new sendatom and add to a clients sendqueue

void irmo_client_sendq_add_new(IrmoClient *client, IrmoObject *object);
void irmo_client_sendq_add_change(IrmoClient *client,
			          IrmoObject *object, int variable);
void irmo_client_sendq_add_destroy(IrmoClient *client, IrmoObject *object);
void irmo_client_sendq_add_method(IrmoClient *client, IrmoMethodData *data);
void irmo_client_sendq_add_sendwindow(IrmoClient *client, int max);

IrmoSendAtom *irmo_client_sendq_pop(IrmoClient *client);

// send entire world state

void irmo_client_sendq_add_state(IrmoClient *client);

#endif /* #ifndef IRMO_SENDATOM_H */

// $Log$
// Revision 1.5  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
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
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.15  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.14  2003/05/20 02:06:06  sdh300
// Add out-of-order execution of stream
//
// Revision 1.13  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.12  2003/04/25 00:40:50  sdh300
// Nullifying of change atoms for out-of-date data in the send window
//
// Revision 1.11  2003/03/18 20:55:47  sdh300
// Initial round trip time measurement
//
// Revision 1.10  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.9  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.8  2003/03/06 21:29:05  sdh300
// On connect, send the entire universe state to the client
//
// Revision 1.7  2003/03/05 15:45:37  sdh300
// Split sendatom type into a seperate type,
// Add missing #includes
//
// Revision 1.6  2003/03/05 15:28:14  sdh300
// Add receive window and extra data for sendatoms in the receive window.
//
// Revision 1.5  2003/03/02 02:10:16  sdh300
// Store send time in atoms
//
// Revision 1.4  2003/02/27 02:07:56  sdh300
// Store sendatom size in structure
// Add 'pop' function to remove atoms from sendq head
//
// Revision 1.3  2003/02/20 18:25:00  sdh300
// Use GQueue instead of a GPtrArray for the send queue
// Initial change/destroy code
//
// Revision 1.2  2003/02/18 20:26:42  sdh300
// Initial send queue building/notification code
//
// Revision 1.1  2003/02/18 18:25:40  sdh300
// Initial queue object code
//

