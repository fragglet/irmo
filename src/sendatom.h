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

#define IRMO_SENDATOM(x) ((IrmoSendAtom *) (x))

typedef struct _IrmoSendAtomClass IrmoSendAtomClass;

typedef struct _IrmoSendAtom IrmoSendAtom;

typedef struct _IrmoNewObjectAtom IrmoNewObjectAtom;
typedef struct _IrmoChangeAtom IrmoChangeAtom;
typedef struct _IrmoDestroyAtom IrmoDestroyAtom;
typedef struct _IrmoMethodAtom IrmoMethodAtom;
typedef struct _IrmoSendWindowAtom IrmoSendWindowAtom;

typedef enum {
	ATOM_NULL,               // null atom for nullified changes
	ATOM_NEW,                // new object 
	ATOM_CHANGE,             // modified object 
	ATOM_DESTROY,            // object destroyed 
	ATOM_METHOD,             // method call
	ATOM_SENDWINDOW,         // set maximum sendwindow size
	NUM_SENDATOM_TYPES,
} IrmoSendAtomType;

#include "netlib.h"

#include "client.h"
#include "if_spec.h"
#include "method.h"
#include "object.h"
#include "packet.h"

#include <glib.h>

typedef gboolean (*IrmoSendAtomVerifyFunc)(IrmoPacket *packet);
typedef IrmoSendAtom *(*IrmoSendAtomReadFunc)(IrmoPacket *packet);
typedef void (*IrmoSendAtomWriteFunc)(IrmoSendAtom *atom, IrmoPacket *packet);
typedef void (*IrmoSendAtomRunFunc)(IrmoSendAtom *atom);
typedef gsize (*IrmoSendAtomLengthFunc)(IrmoSendAtom *atom);
typedef void (*IrmoSendAtomDestroyFunc)(IrmoSendAtom *atom);

struct _IrmoSendAtomClass {
	IrmoSendAtomType type;

	// verify an atom of this type can be read from a packet
	
	IrmoSendAtomVerifyFunc verify;

	// read a new atom of this type from a packet

	IrmoSendAtomReadFunc read;

	// write an atom of this type to a packet

	IrmoSendAtomWriteFunc write;

	// run an atom to apply its effects
	
	IrmoSendAtomRunFunc run;

	// calculate the length of an atom
	
	IrmoSendAtomLengthFunc length;
	
	// destroy an atom of this type

	IrmoSendAtomDestroyFunc destructor;
};

// queue object

struct _IrmoSendAtom {
	IrmoSendAtomClass *klass;

	// client this atom belongs to

	IrmoClient *client;  

	// time atom was last sent

	GTimeVal sendtime;

	// true if this atom was resent

	gboolean resent;

	// length of atom in bytes

	int len;

	// number of this atom in the sequence
	
	int seqnum;
};

struct _IrmoNewObjectAtom {
	IrmoSendAtom sendatom;

	IrmoObjectID id;
	guint classnum;
};

struct _IrmoChangeAtom {
	IrmoSendAtom sendatom;

	gboolean executed;           // atom has been executed
	IrmoObjectID id;
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
};

struct _IrmoDestroyAtom {
	IrmoSendAtom sendatom;

	IrmoObjectID id;
};

struct _IrmoSendWindowAtom {
	IrmoSendAtom sendatom;

	guint max;
};

struct _IrmoMethodAtom {
	IrmoSendAtom sendatom;

	IrmoMethodData method;
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

// atom classes

extern IrmoSendAtomClass irmo_null_atom;
extern IrmoSendAtomClass irmo_newobject_atom;
extern IrmoSendAtomClass irmo_change_atom;
extern IrmoSendAtomClass irmo_destroy_atom;
extern IrmoSendAtomClass irmo_method_atom;
extern IrmoSendAtomClass irmo_sendwindow_atom;

extern IrmoSendAtomClass *irmo_sendatom_types[];

#endif /* #ifndef IRMO_SENDATOM_H */

// $Log$
// Revision 1.11  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.10  2003/11/18 19:32:19  fraggle
// Use GTimeVal instead of struct timeval
//
// Revision 1.9  2003/11/18 18:14:46  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.8  2003/11/17 00:32:28  fraggle
// Rename irmo_objid_t to IrmoObjectID for consistency with other types
//
// Revision 1.7  2003/11/05 04:05:44  fraggle
// Cast functions rather than casting arguments to functions
//
// Revision 1.6  2003/10/14 22:12:50  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
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

