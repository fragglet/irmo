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

#include <irmo/packet.h>

#include "world/object.h"


//
// IrmoDestroyAtom
//
// Announce the destruction of a remote object.
//
// format:
//
// <int16>	object id of object to destroy
//

static int irmo_destroy_atom_verify(IrmoPacket *packet, IrmoClient *client)
{
	unsigned int i;

	if (!client->world)
		return 0;

	// object id

	if (!irmo_packet_readi16(packet, &i))
		return 0;
		
	return 1;
}

static IrmoSendAtom *irmo_destroy_atom_read(IrmoPacket *packet,
                                            IrmoClient *client)
{
	IrmoDestroyAtom *atom;

	atom = irmo_new0(IrmoDestroyAtom, 1);
	atom->sendatom.klass = &irmo_destroy_atom;

	// object id to destroy

	irmo_packet_readi16(packet, &atom->id);

	return IRMO_SENDATOM(atom);
}

static void irmo_destroy_atom_write(IrmoDestroyAtom *atom, IrmoPacket *packet)
{
	irmo_packet_writei16(packet, atom->id);
}

static void irmo_destroy_atom_run(IrmoDestroyAtom *atom)
{
        IrmoClient *client = atom->sendatom.client;
	IrmoObject *obj;

	// sanity check

	obj = irmo_world_get_object_for_id(client->world, atom->id);

	if (!obj) {
		irmo_error_report("client_run_destroy",
				  "destroy object %i, but object does not exist",
				  atom->id);
		return;
	}

	// destroy object. remove from world and call notify functions
	
	irmo_object_internal_destroy(obj, 1, 1);
}

static size_t irmo_destroy_atom_length(IrmoSendAtom *atom)
{
	return 2;
}

IrmoSendAtomClass irmo_destroy_atom = {
	ATOM_DESTROY,
	irmo_destroy_atom_verify,
	irmo_destroy_atom_read,
	(IrmoSendAtomWriteFunc) irmo_destroy_atom_write,
	(IrmoSendAtomRunFunc) irmo_destroy_atom_run,
	(IrmoSendAtomLengthFunc) irmo_destroy_atom_length,
	NULL,
};

