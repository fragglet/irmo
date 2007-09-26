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

#include "arch/sysheaders.h"
#include "base/util.h"
#include "base/error.h"

#include "netbase/netlib.h"
#include "netbase/packet.h"

#include "world/object.h"

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

static int irmo_newobject_atom_verify(IrmoPacket *packet)
{
	unsigned int i;

	if (!packet->client->world)
		return 0;

	// object id

	if (!irmo_packet_readi16(packet, &i))
		return 0;

	// class of new object

	if (!irmo_packet_readi8(packet, &i))
		return 0;

	// check valid class
	
	if (i >= packet->client->world->spec->nclasses)
		return 0;

	return 1;
}

static IrmoSendAtom *irmo_newobject_atom_read(IrmoPacket *packet)
{
	IrmoNewObjectAtom *atom;

	atom = irmo_new0(IrmoNewObjectAtom, 1);
	atom->sendatom.klass = &irmo_newobject_atom;

	// object id of new object
		
	irmo_packet_readi16(packet, &atom->id);

	// class of new object
		
	irmo_packet_readi8(packet, &atom->classnum);

	return IRMO_SENDATOM(atom);
}

static void irmo_newobject_atom_write(IrmoNewObjectAtom *atom,
				      IrmoPacket *packet)
{
	irmo_packet_writei16(packet, atom->id);
	irmo_packet_writei8(packet, atom->classnum);
}

static void irmo_newobject_atom_run(IrmoNewObjectAtom *atom)
{
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

static size_t irmo_newobject_atom_length(IrmoSendAtom *atom)
{
	// object id, class number

	return 2 + 1;
}


IrmoSendAtomClass irmo_newobject_atom = {
	ATOM_NEW,
	irmo_newobject_atom_verify,
	irmo_newobject_atom_read,
	(IrmoSendAtomWriteFunc) irmo_newobject_atom_write,
	(IrmoSendAtomRunFunc) irmo_newobject_atom_run,
	(IrmoSendAtomLengthFunc) irmo_newobject_atom_length,
	NULL,
};

