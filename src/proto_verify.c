// proto_verify: Verify packets are are properly formed before
// parsing

#include "packet.h"

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
	
	for (i=0; result && i<objclass->nvariables; ++i) {
		if (!changed[i])
			continue;
		switch (objclass->variables[i]->type) {
		case TYPE_INT8:
			if (!packet_readi8(packet, &i8))
				result = FALSE;
			break;
		case TYPE_INT16:
			if (!packet_readi16(packet, &i16))
				result = FALSE;
			break;
		case TYPE_INT32:
			if (!packet_readi32(packet, &i32))
				result = FALSE;
			break;
		case TYPE_STRING:
			if (!packet_readstring(packet))
				result = FALSE;
			break;
		}
	}
	
	free(changed);

	return result;
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
	case ATOM_DESTROY:

		// object id

		if (!packet_readi16(packet, &i16))
			return FALSE;
		
		break;
	}

	return TRUE;
}

static gboolean proto_verify_packet_cluster(IrmoPacket *packet)
{
	printf("verify packet\n");
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
		    && atomtype != ATOM_CHANGE && atomtype != ATOM_DESTROY) {
			printf("invalid atom type (%i)\n", atomtype);
			return FALSE;
		}
		printf("%i atoms, %i\n", natoms, atomtype);

		for (i=0; i<natoms; ++i) {
			printf("\tverify atom %i (%i)\n", i, atomtype);
			if (!proto_verify_atom(packet->client, packet,
					       atomtype)) {
				printf("\t\tfailed\n");
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
 
