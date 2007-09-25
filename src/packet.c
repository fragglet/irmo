// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// Copyright (C) 2002-3 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//---------------------------------------------------------------------

//
// Safe packet handling code
//

#include "sysheaders.h"

#include "packet.h"

IrmoPacket *irmo_packet_new(void)
{
	IrmoPacket *packet = irmo_new0(IrmoPacket, 1);

	packet->data_size = 256;
	packet->data = malloc(packet->data_size);
	packet->len = 0;
	packet->pos = 0;

	return packet;
}

void irmo_packet_free(IrmoPacket *packet)
{
	free(packet->data);
	free(packet->src);
	free(packet);
}

static void irmo_packet_resize(IrmoPacket *packet)
{
	// resize exponentially bigger

	packet->data_size *= 2;
	packet->data = realloc(packet->data, packet->data_size);
}

static void irmo_packet_update_len(IrmoPacket *packet)
{
	if (packet->pos > packet->len)
		packet->len = packet->pos;
}

int irmo_packet_writei8(IrmoPacket *packet, unsigned int i)
{
	if (packet->pos + 1 > packet->data_size)
		irmo_packet_resize(packet);
	
	packet->data[packet->pos++] = i;

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_writei16(IrmoPacket *packet, unsigned int i)
{
	if (packet->pos + 2 > packet->data_size)
		irmo_packet_resize(packet);

	packet->data[packet->pos++] = (i >> 8) & 0xff;
	packet->data[packet->pos++] = (i) & 0xff;

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_writei32(IrmoPacket *packet, unsigned int i)
{
	if (packet->pos + 4 > packet->data_size)
		irmo_packet_resize(packet);

	packet->data[packet->pos++] = (i >> 24) & 0xff;
	packet->data[packet->pos++] = (i >> 16) & 0xff;
	packet->data[packet->pos++] = (i >> 8) & 0xff;
	packet->data[packet->pos++] = (i) & 0xff;

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_writestring(IrmoPacket *packet, char *s)
{
	if (packet->pos + strlen(s) + 1 > packet->data_size)
		irmo_packet_resize(packet);

	strcpy((char *) packet->data + packet->pos, s); 
	packet->pos += strlen(s) + 1;

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_readi8(IrmoPacket *packet, unsigned int *i)
{
	if (packet->pos + 1 > packet->len)
		return 0;

	if (i) {
		*i = packet->data[packet->pos];
	}

	packet->pos += 1;

	return 1;
}

int irmo_packet_readi16(IrmoPacket *packet, unsigned int *i)
{
	uint8_t *data;
	
	if (packet->pos + 2 > packet->len)
		return 0;

	data = packet->data + packet->pos;
	
	if (i) {
		*i = (data[0] << 8) + data[1];
	}

	packet->pos += 2;

	return 1;
}

int irmo_packet_readi32(IrmoPacket *packet, unsigned int *i)
{
	uint8_t *data;

	if (packet->pos + 4 > packet->len)
		return 0;
		
	data = packet->data + packet->pos;

	if (i) {
		*i = (data[0] << 24) + (data[1] << 16) 
		   + (data[2] << 8) + data[3];
	}

	packet->pos += 4;

	return 1;
}

char *irmo_packet_readstring(IrmoPacket *packet)
{
	uint8_t *start = packet->data + packet->pos;

	for (; packet->pos < packet->len; ++packet->pos) {
		if (!packet->data[packet->pos]) {
			// skip past the terminating 0
			
			++packet->pos;
			
			return (char *) start;
		}
	}

	// overflowed past the end of the packet
	
	return NULL;
}

int irmo_packet_verify_value(IrmoPacket *packet,
				  IrmoValueType type)
{
	switch (type) {
	case IRMO_TYPE_INT8:
		return irmo_packet_readi8(packet, NULL);
	case IRMO_TYPE_INT16:
		return irmo_packet_readi16(packet, NULL);
	case IRMO_TYPE_INT32:
		return irmo_packet_readi32(packet, NULL);
	case IRMO_TYPE_STRING:
		return irmo_packet_readstring(packet) != NULL;
	}

	return 0;
}

void irmo_packet_read_value(IrmoPacket *packet, IrmoValue *value, 
			    IrmoValueType type)
{
	switch (type) {
	case IRMO_TYPE_INT8:
		irmo_packet_readi8(packet, &value->i);
		break;
	case IRMO_TYPE_INT16:
		irmo_packet_readi16(packet, &value->i);
		break;
	case IRMO_TYPE_INT32:
		irmo_packet_readi32(packet, &value->i);
		break;
	case IRMO_TYPE_STRING:
		value->s = strdup(irmo_packet_readstring(packet));
		break;
	}
}

void irmo_packet_write_value(IrmoPacket *packet, IrmoValue *value,
			     IrmoValueType type)
{
	switch (type) {
	case IRMO_TYPE_INT8:
		irmo_packet_writei8(packet, value->i);
		break;
	case IRMO_TYPE_INT16:
		irmo_packet_writei16(packet, value->i);
		break;
	case IRMO_TYPE_INT32:
		irmo_packet_writei32(packet, value->i);
		break;
	case IRMO_TYPE_STRING:
		irmo_packet_writestring(packet, value->s);
		break;
	}
}

