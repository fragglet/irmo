
#include <sys/time.h>

#include "packet.h"
#include "sendatom.h"

static void proto_add_change_atom(IrmoPacket *packet, IrmoSendAtom *atom)
{
	IrmoObject *obj = atom->data.change.object;
	int bitmap_size;
	int i, j;
	
	// send object id
	
	packet_writei16(packet, obj->id);

	// build and send bitmap

	bitmap_size = (obj->objclass->nvariables + 7) / 8;

	for (i=0; i<bitmap_size; ++i) {
		guint8 b;

		// build a byte at a time

		b = 0;
		
		for (j=0; j<8 && i*8+j<obj->objclass->nvariables; ++j) {
			if (atom->data.change.changed[i*8 + j]) {
				b |= 1 << j;
			}
		}

		packet_writei8(packet, b);
	}

	// send variables

	for (i=0; i<obj->objclass->nvariables; ++i) {

		// check we are sending this variable

		if (!atom->data.change.changed[i])
			continue;

		// todo

		switch (obj->objclass->variables[i]->type) {
		case TYPE_INT8:
			packet_writei8(packet, obj->variables[i].i8);
			break;
		case TYPE_INT16:
			packet_writei16(packet, obj->variables[i].i16);
			break;
		case TYPE_INT32:
			packet_writei32(packet, obj->variables[i].i32);
			break;
		case TYPE_STRING:
			packet_writestring(packet, obj->variables[i].s);
			break;
		}
	}
}

static void proto_add_atom(IrmoPacket *packet, IrmoSendAtom *atom)
{
	switch (atom->type) {
	case ATOM_NEW:
		packet_writei16(packet, atom->data.newobj.id);
		packet_writei8(packet, atom->data.newobj.classnum);
		break;
	case ATOM_CHANGE:
		proto_add_change_atom(packet, atom);
		break;
	case ATOM_DESTROY:
		packet_writei16(packet, atom->data.destroy.id);
		break;
	case ATOM_METHOD:
		break;
	case ATOM_NULL:
		break;
	}
}

IrmoPacket *proto_build_packet(IrmoClient *client, int start, int end)
{
	struct timeval nowtime;
	IrmoPacket *packet;
	int i, n;

	gettimeofday(&nowtime, NULL);
	
	// make a new packet
	
	packet = packet_new(5);

	// header

	packet_writei16(packet, 0);

	// start position in stream
	
	packet_writei16(packet, (client->sendwindow_start + start) & 0xffff);

	// add all sendatoms in the range specified

	for (i=start; i<end;) {

		// group up to 33 sendatoms of the same type together
		// 33 because we send the number of EXTRA sendatoms after
		// the starting one: the first one is implied. after that
		// we can specify up to 32 of the same type that follow
		
		for (n=0; i+n+1<end && n<32; ++n)
			if (client->sendwindow[i+n+1]->type
			 != client->sendwindow[i]->type)
				break;
		
		// store extra count in the low bits, type in the high bits

		packet_writei8(packet,
			       (client->sendwindow[i]->type << 5) | n);

		// add atoms

		for (; n>0; --n, ++i) {
			proto_add_atom(packet, client->sendwindow[i]);

			// store send time in atoms

			client->sendwindow[i]->sendtime = nowtime;
		}
	}

	return packet;
}

// $Log: not supported by cvs2svn $
