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
// IrmoSendWindowAtom
//
// Send a message to set the maximum send window size, to throttle
// bandwidth
//
// format:
//
// <int16>	new send window size in bytes
//

static int irmo_sendwindow_atom_verify(IrmoPacket *packet)
{
	unsigned int i;

	// set maximum sendwindow size

	return irmo_packet_readi16(packet, &i);
}

static IrmoSendAtom *irmo_sendwindow_atom_read(IrmoPacket *packet)
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

//---------------------------------------------------------------------
//
// $Log$
// Revision 1.7  2005/12/25 00:48:29  fraggle
// Use internal memory functions, rather than the glib ones
//
// Revision 1.6  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.5  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.4  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.3  2003/11/18 18:14:47  fraggle
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


