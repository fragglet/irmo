// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id: proto_verify.c,v 1.6 2003-06-09 21:06:52 sdh300 Exp $
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

#include "packet.h"

static inline gboolean proto_verify_field(IrmoPacket *packet,
					  TypeSpec type)
{
	guint8 i8;
	guint16 i16;
	guint32 i32;
	
	switch (type) {
	case TYPE_INT8:
		return packet_readi8(packet, &i8);
	case TYPE_INT16:
		return packet_readi16(packet, &i16);
	case TYPE_INT32:
		return packet_readi32(packet, &i32);
	case TYPE_STRING:
		return packet_readstring(packet) != NULL;
	}
}

static gboolean proto_verify_change_atom(IrmoClient *client,
					 IrmoPacket *packet)
{
	ClassSpec *objclass;
	int i, n, b;
	guint8 i8;
	guint16 i16;
	guint32 i32;
	gboolean result;
	gboolean *changed;
	
	// class

	if (!packet_readi8(packet, &i8))
		return FALSE;

	if (i8 >= client->universe->spec->nclasses)
		return FALSE;

	objclass = client->universe->spec->classes[i8];
	
	// object id

	if (!packet_readi16(packet, &i16))
		return FALSE;

	// read object changed bitmap
	
	result = TRUE;

	changed = g_new0(gboolean, objclass->nvariables);

	for (i=0, n=0; result && i<(objclass->nvariables+7) / 8; ++i) {
		if (!packet_readi8(packet, &i8)) {
			result = FALSE;
			break;
		}

		for (b=0; b<8 && n<objclass->nvariables; ++b, ++n)
			if (i8 & (1 << b))
				changed[n] = TRUE;			
	}

	// check new variable values

	if (result) {
		for (i=0; i<objclass->nvariables; ++i) {
			if (!changed[i])
				continue;
			
			if (!proto_verify_field(packet,
						objclass->variables[i]->type)) {
				result = FALSE;
				break;
			}
		}
	}
	
	free(changed);

	return result;
}
					 
static gboolean proto_verify_method_atom(IrmoClient *client,
					 IrmoPacket *packet)
{
	MethodSpec *method;
	guint8 i8;
	int i;

	// read method index

	if (!packet_readi8(packet, &i8))
		return FALSE;

	// sanity check method index

	if (i8 >= client->server->universe->spec->nmethods)
		return FALSE;

	method = client->server->universe->spec->methods[i8];

	// read arguments

	for (i=0; i<method->narguments; ++i) {
		if (!proto_verify_field(packet, method->arguments[i]->type))
			return FALSE;
	}

	return TRUE;
}

static gboolean proto_verify_atom(IrmoClient *client, IrmoPacket *packet,
				  IrmoSendAtomType type)
{
	guint8 i8;
	guint16 i16;

	switch (type) {
	case ATOM_NULL:
		break;
	case ATOM_NEW:
		// object id

		if (!packet_readi16(packet, &i16))
			return FALSE;

		// class of new object

		if (!packet_readi8(packet, &i8))
			return FALSE;

		// check valid class
		
		if (i8 >= client->universe->spec->nclasses)
			return FALSE;

		break;

	case ATOM_CHANGE:
		return proto_verify_change_atom(client, packet);
	case ATOM_METHOD:
		return proto_verify_method_atom(client, packet);
	case ATOM_DESTROY:

		// object id

		if (!packet_readi16(packet, &i16))
			return FALSE;
		
		break;
	case ATOM_SENDWINDOW:
		// set maximum sendwindow size

		if (!packet_readi16(packet, &i16))
			return FALSE;

		break;
	}

	return TRUE;
}

static gboolean proto_verify_packet_cluster(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	guint16 i16;

	// start position
	
	if (!packet_readi16(packet, &i16))
		return FALSE;

	// read atoms
	
	for (;;) {
		guint8 i8;
		int i;
		int atomtype;
		int natoms;
		
		if (!packet_readi8(packet, &i8))
			break;

		atomtype = (i8 >> 5) & 0x07;
		natoms = (i8 & 0x1f) + 1;

		if (atomtype != ATOM_NULL && atomtype != ATOM_NEW
		    && atomtype != ATOM_CHANGE && atomtype != ATOM_DESTROY
		    && atomtype != ATOM_METHOD
		    && atomtype != ATOM_SENDWINDOW) {
			//printf("invalid atom type (%i)\n", atomtype);
			return FALSE;
		}

		// if this atom is about a change to the remote universe,
		// check we have a remote universe we are expecting changes
		// about
		
		if ((atomtype == ATOM_NEW || atomtype == ATOM_CHANGE
		    || atomtype == ATOM_DESTROY)
		    && !client->universe) {
			return FALSE;
		}

		// same with remote method calls to local universe

		if (atomtype == ATOM_METHOD && !client->server->universe)
			return FALSE;
		
		//printf("%i atoms, %i\n", natoms, atomtype);

		for (i=0; i<natoms; ++i) {
			//printf("\tverify atom %i (%i)\n", i, atomtype);
			if (!proto_verify_atom(client, packet,
					       atomtype)) {
				//printf("\t\tfailed\n");
				return FALSE;
			}
		}
			
	}

	return TRUE;
}

gboolean proto_verify_packet(IrmoPacket *packet)
{
	gboolean result = TRUE;
	guint origpos = packet->pos;
	
	//printf("verify packet\n");
	
	// read ack
	
	if (packet->flags & PACKET_FLAG_ACK) {
		guint16 i16;

		if (!packet_readi16(packet, &i16))
			result = FALSE;
	}

	if (result && packet->flags & PACKET_FLAG_DTA) {
		if (!proto_verify_packet_cluster(packet))
			result = FALSE;
	}

	// restore start position

	packet->pos = origpos;

	return result;		
}

// $Log: not supported by cvs2svn $
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
 
