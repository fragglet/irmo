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

#include <glib.h>
#include <string.h>

#include "object.h"
#include "packet.h"

//
// IrmoNewObjectAtom
//
// Announce the creation of a new object.
//
// format:
// 
// <int16>	object id
// <int8>	object class number
// 

static gboolean irmo_newobject_atom_verify(IrmoPacket *packet)
{
	guint i;

	if (!packet->client->world)
		return FALSE;

	// object id

	if (!irmo_packet_readi16(packet, &i))
		return FALSE;

	// class of new object

	if (!irmo_packet_readi8(packet, &i))
		return FALSE;

	// check valid class
	
	if (i >= packet->client->world->spec->nclasses)
		return FALSE;

	return TRUE;
}

static IrmoSendAtom *irmo_newobject_atom_read(IrmoPacket *packet)
{
	IrmoNewObjectAtom *atom;

	atom = g_new0(IrmoNewObjectAtom, 1);
	atom->sendatom.klass = &irmo_newobject_atom;

	// object id of new object
		
	irmo_packet_readi16(packet, &atom->id);

	// class of new object
		
	irmo_packet_readi8(packet, &atom->classnum);

	return IRMO_SENDATOM(atom);
}

static void irmo_newobject_atom_write(IrmoSendAtom *_atom, 
				      IrmoPacket *packet)
{
	IrmoNewObjectAtom *atom = (IrmoNewObjectAtom *) _atom;

	irmo_packet_writei16(packet, atom->id);
	irmo_packet_writei8(packet, atom->classnum);
}

static void irmo_newobject_atom_run(IrmoSendAtom *_atom)
{
	IrmoNewObjectAtom *atom = (IrmoNewObjectAtom *) _atom;
	IrmoClient *client = atom->sendatom.client;
	IrmoInterfaceSpec *spec = client->world->spec;
	IrmoClass *objclass = spec->classes[atom->classnum];
	
	// sanity check

	if (irmo_world_get_object_for_id(client->world,
					 atom->id)) {
		irmo_error_report("client_run_new",
				  "new object id of %i but an object with "
				  "that id already exists!",
				  atom->id);
		return;
	}

	// create new object
							  
	irmo_object_internal_new(client->world, objclass, atom->id);
}

static gsize irmo_newobject_atom_length(IrmoSendAtom *atom)
{
	// object id, class number

	return 2 + 1;
}


IrmoSendAtomClass irmo_newobject_atom = {
	ATOM_NEW,
	irmo_newobject_atom_verify,
	irmo_newobject_atom_read,
	irmo_newobject_atom_write,
	irmo_newobject_atom_run,
	irmo_newobject_atom_length,
	NULL,
};

//---------------------------------------------------------------------
//
// $Log$
// Revision 1.1  2003/10/22 16:13:10  fraggle
// Split off sendatom classes into separate files
//
//
//---------------------------------------------------------------------


