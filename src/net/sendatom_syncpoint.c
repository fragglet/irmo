//
// Copyright (C) 2009 Simon Howard
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
#include "base/alloc.h"

#include <irmo/packet.h>

#include "world/object.h"

#include "sendatom.h"

//
// Synchronisation point.
//
// When a client connects to a server, the complete world state must
// be sent to the client.  New/change atoms are queued for each object
// in the game world, followed by a synchronization point atom, which
// signals that the complete world state has been sent.
//
// format:
//
//  (empty)
//

static int irmo_sync_point_atom_verify(IrmoPacket *packet, IrmoClient *client)
{
	return 1;
}

static IrmoSendAtom *irmo_sync_point_atom_read(IrmoPacket *packet, IrmoClient *client)
{
	IrmoSendAtom *atom;
	
	atom = irmo_new0(IrmoSendAtom, 1);
	atom->klass = &irmo_sync_point_atom;

	return atom;
}

static void irmo_sync_point_atom_write(IrmoSendAtom *atom, IrmoPacket *packet)
{
	return;
}

static size_t irmo_sync_point_atom_length(IrmoSendAtom *atom)
{
	return 0;
}

// Set the synchronized state if both sides are synchronized.

static void check_synced(IrmoClient *client)
{
        if (client->local_synced && client->remote_synced) {
                irmo_client_set_state(client, IRMO_CLIENT_SYNCHRONIZED);
        }
}

static void irmo_sync_point_atom_run(IrmoSendAtom *atom)
{
	atom->client->local_synced = 1;
        check_synced(atom->client);
}

// Acknowledged by the remote client?

static void irmo_sync_point_atom_acked(IrmoSendAtom *atom)
{
        atom->client->remote_synced = 1;
        check_synced(atom->client);
}

IrmoSendAtomClass irmo_sync_point_atom = {
	ATOM_SYNCPOINT,
	irmo_sync_point_atom_verify,
	irmo_sync_point_atom_read,
	irmo_sync_point_atom_write,
	irmo_sync_point_atom_run,
	irmo_sync_point_atom_length,
        irmo_sync_point_atom_acked,
	NULL,
};

