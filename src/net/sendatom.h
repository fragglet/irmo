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
typedef struct _IrmoSyncPointAtom IrmoSyncPointAtom;

typedef enum {
        ATOM_NULL,               // null atom for nullified changes
        ATOM_NEW,                // new object
        ATOM_CHANGE,             // modified object
        ATOM_DESTROY,            // object destroyed
        ATOM_METHOD,             // method call
        ATOM_SENDWINDOW,         // set maximum sendwindow size
        ATOM_SYNCPOINT,          // synchronization point
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
typedef void (*IrmoSendAtomAckedFunc)(IrmoSendAtom *atom);

//
// All send atoms have a class, which defines the type of the atom and
// has function pointers for various operations.
//

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

        // invoked when the atom is acknowledged received by the
        // remote client that it was sent to.

        IrmoSendAtomAckedFunc acked;

	// destroy an atom of this type

	IrmoSendAtomDestroyFunc destructor;
};

// 
// A send atom is an item to be transmitted over the network.  There 
// are several different types (see below).  These are the common
// properties for all types.
//

struct _IrmoSendAtom {

        // Type of send atom.

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

	size_t len;

	// number of this atom in the sequence
	
	unsigned int seqnum;
};

//
// A "new object" atom is sent when a new object is created in the world
// being served.
//

struct _IrmoNewObjectAtom {

        // Common properties for all atoms.

	IrmoSendAtom sendatom;

        // Object Id for the new object being created.

	IrmoObjectID id;

        // Class of the new object.

	unsigned int classnum;
};

//
// A "change" atom is sent when a change is made to one or more variables
// in an object in the world being served  Changes to multiple variables in
// the same object are grouped together into a single change.
//

struct _IrmoChangeAtom {

        // Common properties for all atoms.

	IrmoSendAtom sendatom;

        // If non-zero, this atom has already been executed.

	int executed;

        // Id of the object that this applies to.

	IrmoObjectID id;

        // Pointer to the object to apply to.

	IrmoObject *object;

	// Number of changed variables in this atom.  If this reaches
        // zero, there are no changes, and the atom can be replaced
        // with a null atom.

	int nchanged;
			
	// Array of flags indicating which variables have changed.
	
	int *changed;

	// Class of the object being changed. this is only
	// used for the receive window.

	IrmoClass *objclass;

	// Array of the new values. this is only used for the
	// receive window. For the send window this is NULL.
			
	IrmoValue *newvalues;
};

// 
// A "destroy" atom is sent when an object is destroyed in the world
// being served.
//

struct _IrmoDestroyAtom {

        // Common properties for all atoms.

	IrmoSendAtom sendatom;

        // Id of the object to destroy.

	IrmoObjectID id;
};

//
// A send window atom is sent to set a limit on the size of the send
// window, to limit the bandwidth used.
//

struct _IrmoSendWindowAtom {

        // Common properties for all atoms.

	IrmoSendAtom sendatom;

        // New send window maximum size.

	unsigned int max;
};

//
// A method atom is sent to invoke a method on the world being served.
// This is "backwards" compared to the other atoms: most atoms come
// "from server to client" describing a change, while this goes 
// "from client to server" to invoke a method.
//

struct _IrmoMethodAtom {
        // Common properties for all atoms.

	IrmoSendAtom sendatom;

        // Method data for invoking the method.

	IrmoMethodData method_data;
};

/*!
 * Free a send atom.
 *
 * @param atom          The atom to free.
 */

void irmo_sendatom_free(IrmoSendAtom *atom);

/*!
 * Nullify a send atom (replace it with a null atom).
 *
 * @param atom          The atom to nullify.
 */

void irmo_sendatom_nullify(IrmoSendAtom *atom);

// atom classes

extern IrmoSendAtomClass irmo_null_atom;
extern IrmoSendAtomClass irmo_newobject_atom;
extern IrmoSendAtomClass irmo_change_atom;
extern IrmoSendAtomClass irmo_destroy_atom;
extern IrmoSendAtomClass irmo_method_atom;
extern IrmoSendAtomClass irmo_sendwindow_atom;
extern IrmoSendAtomClass irmo_sync_point_atom;

extern IrmoSendAtomClass *irmo_sendatom_types[];

#endif /* #ifndef IRMO_NET_SENDATOM_H */

