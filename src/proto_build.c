#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "packet.h"
#include "protocol.h"
#include "sendatom.h"

static void proto_add_change_atom(IrmoPacket *packet, IrmoSendAtom *atom)
{
	IrmoObject *obj = atom->data.change.object;
	int bitmap_size;
	int i, j;

	// include the object class number
	// this is neccesary otherwise the packet can be ambiguous to
	// decode (if we receive a change atom for an object which has
	// not yet been received, for example)

	packet_writei8(packet, obj->objclass->index);
	
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
	// always send last-acked point 
	
	packet_writei16(packet, PACKET_FLAG_ACK|PACKET_FLAG_DTA);
	
	// in sending stream positions we only send the low 16
	// bits. the higher bits can be implied by their current
	// position
	
	// last acked point
	
	packet_writei16(packet, client->recvwindow_start & 0xffff);
	client->need_ack = FALSE;
	
	// start position in stream
	
	packet_writei16(packet, (client->sendwindow_start + start) & 0xffff);

	//printf("-- build_packet: range %i-%i\n", start,end);
	// add all sendatoms in the range specified

	for (i=start; i<=end;) {

		// group up to 32 sendatoms of the same type together
		// we send the number of EXTRA sendatoms after
		// the starting one: the first one is implied. after that
		// we can specify up to 31 of the same type that follow
		
		for (n=1; i+n<=end && n<=32; ++n)
			if (client->sendwindow[i+n]->type
			 != client->sendwindow[i]->type)
				break;

		//printf("-- build_packet: group length %i, type %i\n",
		//n, client->sendwindow[i]->type);
		       

		// store extra count in the low bits, type in the high bits

		packet_writei8(packet,
			       (client->sendwindow[i]->type << 5) | (n-1));

		// add atoms

		for (; n>0; --n, ++i) {
			proto_add_atom(packet, client->sendwindow[i]);

			// store send time in atoms

			client->sendwindow[i]->sendtime = nowtime;
		}
	}

	return packet;
}

static inline int timeval_cmp(struct timeval *a, struct timeval *b)
{
	if (a->tv_sec == b->tv_sec) {
		return a->tv_usec < b->tv_usec ? -1 :
			a->tv_usec > b->tv_usec ? 1 : 0;
	}
	
	return a->tv_sec < b->tv_sec ? -1 : 1;
}

// data gets pumped into the sendwindow until it passes this threshold
// size, then no more is added

// this is currently static. TODO: modify based on network conditions
// (eg. bigger for high bandwidth connections, smaller for low bandwidth)

#define SENDWINDOW_THRESHOLD 4096

static void proto_pump_client(IrmoClient *client)
{
	int current_size = 0;
	int i;
	
	// if queue is already empty, this is a nonissue
	
	if (g_queue_is_empty(client->sendq))
		return;

	for (i=0; i<client->sendwindow_size; ++i)
		current_size += client->sendwindow[i]->len;

	// adding things in until we run out of space or atoms to add
	
	while (current_size < SENDWINDOW_THRESHOLD
	       && !g_queue_is_empty(client->sendq)
	       && client->sendwindow_size < MAX_SENDWINDOW) {
		IrmoSendAtom *atom;

		// pop another from the sendq and add to the sendwindow

		atom = irmo_client_sendq_pop(client);		
		atom->sendtime.tv_sec = 0;
		
		client->sendwindow[client->sendwindow_size++] = atom;

		// keep track of size
		
		current_size += atom->len;
	}
}

// size of each packet; when size of atoms passes this threshold
// no more atoms are added to it

#define PACKET_THRESHOLD 1024

// time before packets time out and are resent (in seconds)
// this is currently static; TODO: calculate the optimum timeout
// time from the average round trip time (jacobsons algorithm)

#define TIMEOUT_LENGTH 2

void proto_run_client(IrmoClient *client)
{
	struct timeval timeout_time;
	int i;
	
	proto_pump_client(client);

	gettimeofday(&timeout_time, NULL);
	timeout_time.tv_sec -= TIMEOUT_LENGTH;

	//printf("-- %i\n", timeout_time.tv_sec);

	//for (i=0; i<client->sendwindow_size; ++i) {
	//printf("%i: %i\n", i, client->sendwindow[i]->sendtime.tv_sec);
	//}

	for (i=0; i<client->sendwindow_size; ) {
		IrmoPacket *packet;
		int len;
		int start;

		//printf("find start\n");
		
		// search forward until we find the start of a block

		while (i<client->sendwindow_size
		       && timeval_cmp(&client->sendwindow[i]->sendtime,
				      &timeout_time) > 0) {
			//printf("atom %i not expired yet\n", i);
			++i;
		}

		// no more atoms?
		
		if (i >= client->sendwindow_size)
			break;

		//printf("find end\n");
		
		start = i;
		len = 0;
		
		while (i<client->sendwindow_size
		       && timeval_cmp(&client->sendwindow[i]->sendtime,
				      &timeout_time) <= 0
		       && len < PACKET_THRESHOLD) {
			//printf("atom %i out of date\n", i);
			len += client->sendwindow[i]->len;
			++i;
		}

		//printf("packet %i->%i\n", start, i-1);
		// build a packet 

		packet = proto_build_packet(client, start, i-1);

		//printf("sendpacket: %i->%i\n", start, i-1);
		
		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		//printf("free packet\n");
		
		// finished using packet
		
		packet_free(packet);
	}

	//printf("finished\n");

	// possibly we need to send an ack for something we have received
	// if we have nothing in our send window to send, we still need
	// to send an ack back

	if (client->need_ack) {
		IrmoPacket *packet;

		//printf("send ack to client\n");

		packet = packet_new(4);

		// only ack flag is sent, not dta as there is no data
		
		packet_writei16(packet, PACKET_FLAG_ACK);
		packet_writei16(packet, client->recvwindow_start & 0xffff);
		client->need_ack = FALSE;

		// send packet

		irmo_socket_sendpacket(client->server->socket,
				       client->addr,
				       packet);

		packet_free(packet);
	}
}

// $Log: not supported by cvs2svn $
// Revision 1.7  2003/03/07 12:31:51  sdh300
// Add protocol.h
//
// Revision 1.6  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.5  2003/03/06 20:15:25  sdh300
// Initial ack code
//
// Revision 1.4  2003/03/05 17:33:57  sdh300
// Fix length threshold checking (len variable was uninitialised)
//
// Revision 1.3  2003/03/05 15:32:21  sdh300
// Add object class to change atoms to make their coding in packets
// unambiguous.
//
// Revision 1.2  2003/03/03 22:14:44  sdh300
// Initial window construction and sending of packets
//
// Revision 1.1  2003/03/02 02:12:28  sdh300
// Initial packet building code
//
