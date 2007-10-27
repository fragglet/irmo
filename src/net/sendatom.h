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

#ifndef IRMO_NET_SENDATOM_H
#define IRMO_NET_SENDATOM_H

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

// time value which indicates an atom has not yet been sent

#define IRMO_ATOM_UNSENT UINT_MAX

#include <limits.h>

#include "interface/interface.h"
#include "world/method.h"
#include "world/object.h"
#include <irmo/packet.h>

#include "client.h"

typedef int (*IrmoSendAtomVerifyFunc)(IrmoPacket *packet, IrmoClient *client);
typedef IrmoSendAtom *(*IrmoSendAtomReadFunc)(IrmoPacket *packet, IrmoClient *clientt);
typedef void (*IrmoSendAtomWriteFunc)(IrmoSendAtom *atom, IrmoPacket *packet);
typedef void (*IrmoSendAtomRunFunc)(IrmoSendAtom *atom);
typedef size_t (*IrmoSendAtomLengthFunc)(IrmoSendAtom *atom);
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
        // if this is equal to IRMO_ATOM_UNSENT, it has not yet 
        // been transmitted.

	unsigned int sendtime;

	// true if this atom was resent

	int resent;

	// length of atom in bytes

	int len;

	// number of this atom in the sequence
	
	int seqnum;
};

struct _IrmoNewObjectAtom {
	IrmoSendAtom sendatom;

	IrmoObjectID id;
	unsigned int classnum;
};

struct _IrmoChangeAtom {
	IrmoSendAtom sendatom;

	int executed;           // atom has been executed
	IrmoObjectID id;
	IrmoObject *object;

	// count of number of changed variables in this atom
			
	int nchanged;
			
	// array saying which have changed
	
	int *changed;

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

	unsigned int max;
};

struct _IrmoMethodAtom {
	IrmoSendAtom sendatom;

	IrmoMethodData method_data;
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

#endif /* #ifndef IRMO_NET_SENDATOM_H */

