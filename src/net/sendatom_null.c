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

#include "netbase/netlib.h"
#include "netbase/packet.h"

#include "world/object.h"

//
// Null atom.
//
// does nothing.
//
// format:
// 
//  (empty)
//

static int irmo_null_atom_verify(IrmoPacket *packet)
{
	return 1;
}

static IrmoSendAtom *irmo_null_atom_read(IrmoPacket *packet)
{
	IrmoSendAtom *atom;
	
	atom = irmo_new0(IrmoSendAtom, 1);
	atom->klass = &irmo_null_atom;

	return atom;
}

static void irmo_null_atom_write(IrmoSendAtom *atom, IrmoPacket *packet)
{
	return;
}

static size_t irmo_null_atom_length(IrmoSendAtom *atom)
{
	return 0;
}

static void irmo_null_atom_run(IrmoSendAtom *atom)
{
	// does nothing
}

IrmoSendAtomClass irmo_null_atom = {
	ATOM_NULL,
	irmo_null_atom_verify,
	irmo_null_atom_read,
	irmo_null_atom_write,
	irmo_null_atom_run,
	irmo_null_atom_length,
	NULL,
};

