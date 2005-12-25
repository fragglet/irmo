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

