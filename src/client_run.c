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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "object.h"
#include "sendatom.h"
#include "universe.h"

static void client_run_new(IrmoClient *client, IrmoSendAtom *atom)
{
	IrmoInterfaceSpec *spec = client->universe->spec;
	ClassSpec *objclass = spec->classes[atom->data.newobj.classnum];
	
	// sanity check

	if (irmo_universe_get_object_for_id(client->universe,
					    atom->data.newobj.id)) {
		irmo_error_report("client_run_new",
				  "new object id of %i but an object with "
				  "that id already exists!",
				  atom->data.newobj.id);
		return;
	}

	// create new object
							  
	irmo_object_internal_new(client->universe, objclass,
				 atom->data.newobj.id);
	
}

static void client_run_change(IrmoClient *client, IrmoSendAtom *atom,
			      int seq)
{
	IrmoObject *obj;
	ClassSpec *objclass;
	IrmoVariable *newvalues;
	int i;

	if (atom->data.change.executed)
		return;
	
	// sanity checks

	obj = irmo_universe_get_object_for_id(client->universe,
					      atom->data.change.id);

	// if these fail, it is possibly because of dependencies on
	// previous atoms in the stream
	
	if (!obj)
		return;
	if (obj->objclass != atom->data.change.objclass)
		return;	       

	objclass = obj->objclass;
	newvalues = atom->data.change.newvalues;
	
	// run through variables and apply changes
	
	for (i=0; i<obj->objclass->nvariables; ++i) {

		// not changed?
		
		if (!atom->data.change.changed[i])
			continue;

		// check if a newer change to this atom has been run
		// do not apply older changes
		// dont run the same atom twice (could conceivably
		// happen with resends)
		
		if (seq <= obj->variable_time[i])
			continue;
		
		// apply change

		switch (objclass->variables[i]->type) {
		case TYPE_INT8:
			obj->variables[i].i8 = newvalues[i].i8;
			break;
		case TYPE_INT16:
			obj->variables[i].i16 = newvalues[i].i16;
			break;
		case TYPE_INT32:
			obj->variables[i].i32 = newvalues[i].i32;
			break;
		case TYPE_STRING:
			free(obj->variables[i].s);
			obj->variables[i].s = strdup(newvalues[i].s);
			break;
		}

		irmo_object_set_raise(obj, i);

		obj->variable_time[i] = seq;
	}

	// mark as executed
	
	atom->data.change.executed = TRUE;
}

static void client_run_destroy(IrmoClient *client, IrmoSendAtom *atom)
{
	IrmoObject *obj;

	// sanity check

	obj = irmo_universe_get_object_for_id(client->universe,
					      atom->data.destroy.id);

	if (!obj) {
		irmo_error_report("client_run_destroy",
				  "destroy object %i, but object does not exist",
				  atom->data.destroy.id);
		return;
	}

	// destroy object. remove from universe and call notify functions
	
	irmo_object_internal_destroy(obj, TRUE, TRUE);
}

static void client_run_method(IrmoClient *client, IrmoSendAtom *atom)
{
	atom->data.method.src = client;
	
	irmo_method_invoke(client->server->universe, &atom->data.method);
}

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
		
		if (atom && atom->type == ATOM_CHANGE)
			client_run_change(client, atom,
					  client->recvwindow_start + i);
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

		switch (atom->type) {
		case ATOM_NULL:
			break;
		case ATOM_NEW:
			client_run_new(client, atom);
			break;
		case ATOM_CHANGE:
			client_run_change(client, atom,
					  client->recvwindow_start + i);
			break;
		case ATOM_DESTROY:
			client_run_destroy(client, atom);
			break;
		case ATOM_METHOD:
			client_run_method(client, atom);
			break;
		case ATOM_SENDWINDOW:
			client->remote_sendwindow_max
				= atom->data.sendwindow.max;
			break;
		}

		sendatom_free(atom);
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
