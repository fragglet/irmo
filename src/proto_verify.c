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

