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

#include <string.h>

#include "netlib.h"

#include "error.h"
#include "object.h"
#include "packet.h"

//
// IrmoDestroyAtom
//
// Announce the destruction of a remote object.
//
// format:
//
// <int16>	object id of object to destroy
//

static gboolean irmo_destroy_atom_verify(IrmoPacket *packet)
{
	guint i;

	if (!packet->client->world)
		return FALSE;

	// object id

	if (!irmo_packet_readi16(packet, &i))
		return FALSE;
		
	return TRUE;
}

static IrmoSendAtom *irmo_destroy_atom_read(IrmoPacket *packet)
{
	IrmoDestroyAtom *atom;

	atom = g_new0(IrmoDestroyAtom, 1);
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
	
	irmo_object_internal_destroy(obj, TRUE, TRUE);
}

static gsize irmo_destroy_atom_length(IrmoSendAtom *atom)
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

//---------------------------------------------------------------------
//
// $Log$
// Revision 1.4  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.3  2003/11/18 18:14:46  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.2  2003/11/05 04:05:44  fraggle
// Cast functions rather than casting arguments to functions
//
// Revision 1.1  2003/10/22 16:13:10  fraggle
// Split off sendatom classes into separate files
//
//
//---------------------------------------------------------------------


