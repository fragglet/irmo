// parsing of received packets
// this code runs on the assumption that the packets being given to
// it are well formed and valid; it does not do any checking on the data
// received.
//
// TODO: proto_verify.c for verifification of packets before they are
// parsed.

#include "packet.h"
#include "sendatom.h"

// only the low 16 bits of the stream position is sent
// therefore we must expand positions we get based on the
// current position

static inline int get_stream_position(int current, int low)
{
	int newpos = (current & ~0xffff) | low;

	// if new position is greater than 32768 behind the
	// current position, we have probably wrapped around
	// similarly wrap around the other way
	
	if (current - newpos > 0x8000)
		newpos += 0x10000;
	else if (newpos - current > 0x8000)
		newpos -= 0x10000;

	return newpos;
}

static inline void proto_parse_change_atom(IrmoClient *client,
					   IrmoPacket *packet,
					   IrmoSendAtom *atom)
{
	ClassSpec *objclass;
	gboolean *changed;
	IrmoVariable *newvalues;
	int i, b, n;
	guint8 i8;
	guint16 i16;

	// read class
	
	packet_readi8(packet, &i8);

	printf("object class number: %i\n", i8);
	objclass = client->universe->spec->classes[i8];

	// read object id
	
	packet_readi16(packet, &i16);

	atom->data.change.id = i16;
	
	// read the changed object bitmap

	changed = g_new0(gboolean, objclass->nvariables);
	atom->data.change.changed = changed;
	
	for (i=0, n=0; i<(objclass->nvariables+7) / 8; ++i) {

		// read the bits out of this byte in the bitmap into the
		// changed array
		
		packet_readi8(packet, &i8);

		for (b=0; b<8 && n<objclass->nvariables; ++b,++n)
			if (i8 & (1 << b))
				changed[n] = TRUE;
	}

	// read the new values

	newvalues = g_new0(IrmoVariable, objclass->nvariables);
	atom->data.change.newvalues = newvalues;

	for (i=0; i<objclass->nvariables; ++i) {
		if (!changed[i])
			continue;
		switch (objclass->variables[i]->type) {
		case TYPE_INT8:
			packet_readi8(packet, &newvalues[i].i8);
			break;
		case TYPE_INT16:
			packet_readi16(packet, &newvalues[i].i16);
			break;
		case TYPE_INT32:
			packet_readi32(packet, &newvalues[i].i32);
			break;
		case TYPE_STRING:
			newvalues[i].s = packet_readstring(packet);
			break;
		}
	}

}

static IrmoSendAtom *proto_parse_atom(IrmoClient *client, IrmoPacket *packet,
				      IrmoSendAtomType type)
{
	IrmoSendAtom *atom;
	guint8 i8;
	guint16 i16;

	atom = g_new0(IrmoSendAtom, 1);
	atom->type = type;

	switch (type) {
	case ATOM_NULL:
		break;
	case ATOM_NEW:
		// object id of new object
		
		packet_readi16(packet, &i16);
		atom->data.newobj.id = i16;

		// class of new object
		
		packet_readi8(packet, &i8);
		atom->data.newobj.classnum = i8;
		break;

	case ATOM_CHANGE:
		proto_parse_change_atom(client, packet, atom);
		break;
	case ATOM_DESTROY:
		// object id to destroy

		packet_readi16(packet, &i16);
		atom->data.destroy.id = i16;

		break;
	default:
		atom->type = ATOM_NULL;
		break;
	}

	return atom;
}

static void proto_parse_insert_atom(IrmoClient *client,
				    IrmoSendAtom *atom,
				    int seq)
{
	int index = seq - client->recvwindow_start;
	int i;

	// increase the receive window size if neccesary
	
	if (index >= client->recvwindow_size) {
		int newsize = index+1;

		printf("resize recv window to %i\n", newsize);
		
		client->recvwindow
			= g_renew(IrmoSendAtom *,
				  client->recvwindow,
				  newsize);

		for (i=client->recvwindow_size; i<newsize; ++i)
			client->recvwindow[i] = NULL;

		client->recvwindow_size = newsize;
	}

	// delete old sendatoms in the same position (assume
	// new retransmitted atoms are more up to date)
	
	if (client->recvwindow[index])
		sendatom_free(client->recvwindow[index]);

	client->recvwindow[index] = atom;
}

static void proto_parse_packet_cluster(IrmoClient *client, IrmoPacket *packet)
{
	guint8 i8;
	guint16 i16;
	int i, seq;
	int start;

	// get the start position
	
	packet_readi16(packet, &i16);

	start = get_stream_position(client->recvwindow_start, i16);

	printf("stream position: %i->%i\n", i16, start);
	
	for (seq=start;;) {
		IrmoSendAtomType atomtype;
		int natoms;
		
		// read type/count byte
		// if none, end of packet
		
		if (!packet_readi8(packet, &i8))
			break;

		atomtype = (i8 >> 5) & 0x07;
		natoms = (i8 & 0x1f) + 1;

		printf("%i atoms, type %i\n", natoms, atomtype);
		
		for (i=0; i<natoms; ++i, ++seq) {
			IrmoSendAtom *atom;

			atom = proto_parse_atom(client, packet, atomtype);

			// too old?
			
			if (seq < client->recvwindow_start) {
				sendatom_free(atom);
				continue;
			}

			// insert atom into receive window

			proto_parse_insert_atom(client, atom, seq);
		}
	}
}

void proto_parse_packet(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;

	printf("parse packet\n");
	
	// todo: parse ack field

	proto_parse_packet_cluster(client, packet);

	
}

// $Log: not supported by cvs2svn $
