// parsing of received packets
// this code runs on the assumption that the packets being given to
// it are well formed and valid; it does not do any checking on the data
// received.
//
// TODO: proto_verify.c for verifification of packets before they are
// parsed.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "packet.h"
#include "protocol.h"
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

static inline void proto_parse_field(IrmoPacket *packet,
				     IrmoVariable *value,
				     TypeSpec type)
{
	switch (type) {
	case TYPE_INT8:
		packet_readi8(packet, &value->i8);
		break;
	case TYPE_INT16:
		packet_readi16(packet, &value->i16);
		break;
	case TYPE_INT32:
		packet_readi32(packet, &value->i32);
		break;
	case TYPE_STRING:
		value->s = strdup(packet_readstring(packet));
		break;
	}
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

	objclass = client->universe->spec->classes[i8];
	atom->data.change.objclass = objclass;

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

		proto_parse_field(packet, &newvalues[i],
				  objclass->variables[i]->type);
	}

}

static IrmoSendAtom *proto_parse_method_atom(IrmoClient *client,
					     IrmoPacket *packet,
					     IrmoSendAtom *atom)
{
	MethodSpec *method;
	guint8 i8;
	int i;
	
	// read method number
	
	packet_readi8(packet, &i8);
	method = client->server->universe->spec->methods[i8];
	
	atom->data.method.spec = method;

	// read arguments
	
	atom->data.method.args = g_new0(IrmoVariable, method->narguments);

	for (i=0; i<method->narguments; ++i) {
		proto_parse_field(packet, &atom->data.method.args[i],
				  method->arguments[i]->type);
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
	case ATOM_METHOD:
		proto_parse_method_atom(client, packet, atom);
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

	//printf("stream position: %i->%i\n", i16, start);
	
	for (seq=start;;) {
		IrmoSendAtomType atomtype;
		int natoms;
		
		// read type/count byte
		// if none, end of packet
		
		if (!packet_readi8(packet, &i8))
			break;

		atomtype = (i8 >> 5) & 0x07;
		natoms = (i8 & 0x1f) + 1;

		//printf("%i atoms, type %i\n", natoms, atomtype);
		
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

static void proto_parse_ack(IrmoClient *client, int ack)
{
	int seq;
	int relative;
	int i;

	//printf("got an ack: %i\n", ack);
	
	// extrapolate the high bits from the low 16 bits

	seq = get_stream_position(client->sendwindow_start, ack);

	// get position in sendwindow array, relative to the start

	relative = seq - client->sendwindow_start;

	if (relative <= 0) {
		// already acked this far

		return;
	}

	if (relative > client->sendwindow_size) {
		// bogus ack
		// we havent even sent this far in the stream yet

		fprintf(stderr, "proto_parse_ack: bogus ack (%i < %i < %i)\n",
			client->sendwindow_start, seq,
			client->sendwindow_start + client->sendwindow_size);
		return;
	}
	
	// need to move the sendwindow along
	// destroy the atoms in the area acked
	
	for (i=0; i<relative; ++i)
		sendatom_free(client->sendwindow[i]);

	memcpy(client->sendwindow,
	       client->sendwindow + relative,
	       sizeof(*client->sendwindow)
	         * (client->sendwindow_size - relative));

	client->sendwindow_start += relative;
	client->sendwindow_size -= relative;
}

void proto_parse_packet(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;

	// verify packet before parsing for security
	
	if (!proto_verify_packet(packet)) {
		fprintf(stderr,
			"proto_parse_packet: Dropped packet (failed security "
			"verification)\n");
		return;
	}
	
	// read ack field if there is one

	if (packet->flags & PACKET_FLAG_ACK) {
		guint16 i16;

		packet_readi16(packet, &i16);

		proto_parse_ack(client, i16);
	}

	if (packet->flags & PACKET_FLAG_DTA) {
		proto_parse_packet_cluster(client, packet);

		irmo_client_run_recvwindow(client);

		// need to send an ack
		// do this even if we already got the data received, incase
		// the ack was lost and this is why the client is
		// retransmitting
		
		client->need_ack = TRUE;
	}
}

// $Log: not supported by cvs2svn $
// Revision 1.9  2003/03/14 01:07:23  sdh300
// Initial packet verification code
//
// Revision 1.8  2003/03/12 18:59:26  sdh300
// Remove/comment out some debug messages
//
// Revision 1.7  2003/03/07 12:31:51  sdh300
// Add protocol.h
//
// Revision 1.6  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.5  2003/03/06 20:42:06  sdh300
// Fix a couple of bugs in received change atom creation
//
// Revision 1.4  2003/03/06 20:15:25  sdh300
// Initial ack code
//
// Revision 1.3  2003/03/06 19:23:14  sdh300
// Add initial code to run through the atoms in the send window
//
// Revision 1.2  2003/03/06 18:08:14  sdh300
// Add missing cvs log tag
//
