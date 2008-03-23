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
#include "base/alloc.h"

#include <irmo/packet.h>

#include "world/object.h"

#include "sendatom.h"

//
// IrmoSendWindowAtom
//
// Send a message to set the maximum send window size, to throttle
// bandwidth
//
// format:
//
// <int16>	new send window size in bytes
//

static int irmo_sendwindow_atom_verify(IrmoPacket *packet, IrmoClient *client)
{
	unsigned int i;

	// set maximum sendwindow size

	return irmo_packet_readi16(packet, &i);
}

static IrmoSendAtom *irmo_sendwindow_atom_read(IrmoPacket *packet,
                                               IrmoClient *client)
{
	IrmoSendWindowAtom *atom;

	atom = irmo_new0(IrmoSendWindowAtom, 1);
	atom->sendatom.klass = &irmo_sendwindow_atom;
	
	// read window advertisement

	irmo_packet_readi16(packet, &atom->max);

	return IRMO_SENDATOM(atom);
}

static void irmo_sendwindow_atom_write(IrmoSendWindowAtom *atom,
				       IrmoPacket *packet)
{
	irmo_packet_writei16(packet, atom->max);
}

static void irmo_sendwindow_atom_run(IrmoSendWindowAtom *atom)
{
	IrmoClient *client = atom->sendatom.client;

	client->remote_sendwindow_max = atom->max;
}

static size_t irmo_sendwindow_atom_length(IrmoSendAtom *atom)
{
	return 2;
}

IrmoSendAtomClass irmo_sendwindow_atom = {
	ATOM_SENDWINDOW,
	irmo_sendwindow_atom_verify,
	irmo_sendwindow_atom_read,
	(IrmoSendAtomWriteFunc) irmo_sendwindow_atom_write,
	(IrmoSendAtomRunFunc) irmo_sendwindow_atom_run,
	(IrmoSendAtomLengthFunc) irmo_sendwindow_atom_length,
	NULL,
};

