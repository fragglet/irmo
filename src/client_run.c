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
// run sendatoms in receive window
//

#include "sysheaders.h"

#include "client.h"
#include "object.h"
#include "sendatom.h"
#include "world.h"

// preexec receive window, run change atoms where possible,
// asyncronously

void irmo_client_run_preexec(IrmoClient *client, int start, int end)
{
	int i;

	if (start < 0)
		start = 0;
	
	//printf("preexec %i->%i\n", start, end);
	
	for (i=start; i<end; ++i) {
		IrmoSendAtom *atom = client->recvwindow[i];

		// only run change atoms
		
		if (atom && atom->klass == &irmo_change_atom)
			irmo_change_atom.run(atom);
	}
}

// run through receive window

void irmo_client_run_recvwindow(IrmoClient *client)
{
	int i, n;

	// nothing to run?
	
	if (!client->recvwindow[0])
		return;
	
	// run as many from the start as possible
	
	for (i=0;
	     i<client->recvwindow_size && client->recvwindow[i];
	     ++i) {
		IrmoSendAtom *atom = client->recvwindow[i];

		atom->klass->run(atom);

		irmo_sendatom_free(atom);
	}
	
	// move recvwindow along

	client->recvwindow_start += i;
	
	memcpy(client->recvwindow,
	       client->recvwindow + i,
	       sizeof(*client->recvwindow) * (client->recvwindow_size-i));

	// clear the end
	
	for (n=client->recvwindow_size-i; n<client->recvwindow_size; ++n)
		client->recvwindow[n] = NULL;
}

// $Log$
// Revision 1.9  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.8  2003/10/14 22:12:49  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.7  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.6  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.5  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.4  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.3  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.2  2003/07/24 01:25:27  fraggle
// Add an error reporting API
//
// Revision 1.1.1.1  2003/06/09 21:33:23  fraggle
// Initial sourceforge import
//
// Revision 1.10  2003/06/09 21:06:50  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.9  2003/05/20 02:06:06  sdh300
// Add out-of-order execution of stream
//
// Revision 1.8  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.7  2003/03/17 16:49:44  sdh300
// Always include source as IrmoConnections are now IrmoClients
//
// Revision 1.6  2003/03/16 17:38:45  sdh300
// Fix bug with receive window spuriously advancing, caused by
// improperly clearing the end of the recvwindow when advancing it
//
// Revision 1.5  2003/03/16 01:54:23  sdh300
// Method calls over network protocol
//
// Revision 1.4  2003/03/12 18:59:25  sdh300
// Remove/comment out some debug messages
//
// Revision 1.3  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.2  2003/03/06 19:33:50  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.1  2003/03/06 19:23:13  sdh300
// Add initial code to run through the atoms in the send window
//
