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

#include "sysheaders.h"

#include "netlib.h"
#include "object.h"
#include "packet.h"

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

//---------------------------------------------------------------------
//
// $Log$
// Revision 1.5  2005/12/25 00:48:29  fraggle
// Use internal memory functions, rather than the glib ones
//
// Revision 1.4  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.3  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.2  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.1  2003/10/22 16:13:10  fraggle
// Split off sendatom classes into separate files
//
//
//---------------------------------------------------------------------


