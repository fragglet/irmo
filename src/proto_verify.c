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

// proto_verify: Verify packets are are properly formed before
// parsing

#include "sysheaders.h"

#include "packet.h"

static int proto_verify_packet_cluster(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	unsigned int i;

	// start position
	
	if (!irmo_packet_readi16(packet, &i))
		return 0;

	// read atoms
	
	for (;;) {
		IrmoSendAtomClass *klass;
		int atomtype;
		unsigned int natoms;
		
		if (!irmo_packet_readi8(packet, &i))
			break;

		atomtype = (i >> 5) & 0x07;
		natoms = (i & 0x1f) + 1;

		if (atomtype >= NUM_SENDATOM_TYPES) {
			//printf("invalid atom type (%i)\n", atomtype);
			return 0;
		}

		klass = irmo_sendatom_types[atomtype];

		//printf("%i atoms, %i\n", natoms, atomtype);

		for (i=0; i<natoms; ++i) {
			if (!klass->verify(packet)) {
				//printf("\t\tfailed\n");
				return 0;
			}
		}
			
	}

	return 1;
}

int irmo_proto_verify_packet(IrmoPacket *packet)
{
	int result = 1;
	unsigned int origpos = packet->pos;
	
	//printf("verify packet\n");
	
	// read ack
	
	if (packet->flags & PACKET_FLAG_ACK) {
		unsigned int ack;

		if (!irmo_packet_readi16(packet, &ack))
			result = 0;
	}

	if (result && packet->flags & PACKET_FLAG_DTA) {
		if (!proto_verify_packet_cluster(packet))
			result = 0;
	}

	// restore start position

	packet->pos = origpos;

	return result;		
}

// $Log$
// Revision 1.12  2005/12/24 00:15:59  fraggle
// Use the C99 "uintN_t" standard integer types rather than the glib
// guint types.
//
// Revision 1.11  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.10  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.9  2003/11/18 18:14:46  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.8  2003/10/14 22:12:50  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.7  2003/09/12 11:30:26  fraggle
// Rename IrmoVarType to IrmoValueType to be orthogonal to IrmoValue
//
// Revision 1.6  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.5  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.4  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.3  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.2  2003/08/18 01:23:14  fraggle
// Use G_INLINE_FUNC instead of inline for portable inline function support
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.6  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.5  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.4  2003/03/16 01:59:21  sdh300
// Security checks incase of changes/method calls to nonexistent universes
//
// Revision 1.3  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.2  2003/03/14 18:30:24  sdh300
// Fix verification
//
// Revision 1.1  2003/03/14 01:07:23  sdh300
// Initial packet verification code
//
 
