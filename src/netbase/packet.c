//
// Copyright (C) 2002-2008 Simon Howard
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

//
// Safe packet handling code
//

#include "arch/sysheaders.h"
#include "base/alloc.h"
#include "base/assert.h"

#include <irmo/packet.h>

struct _IrmoPacket {
        // Packet data
	uint8_t *data;

        // If true, the data buffer is 'owned' by this packet and
        // should be freed with the packet.
        int data_owned;

        // Size of the buffer (malloced)
	unsigned int data_size;

        // Length of used data in the buffer
	size_t len;

        // Current position in packet
	size_t pos;
};

IrmoPacket *irmo_packet_new(void)
{
	IrmoPacket *packet = irmo_new0(IrmoPacket, 1);

	packet->data_size = 256;
	packet->data = irmo_malloc0(packet->data_size);
        packet->data_owned = 1;
	packet->len = 0;
	packet->pos = 0;

	return packet;
}

IrmoPacket *irmo_packet_new_from(uint8_t *data, unsigned int data_len)
{
        IrmoPacket *packet;

        irmo_return_val_if_fail(data != NULL, NULL);

        packet = irmo_new0(IrmoPacket, 1);

        packet->data = data;
        packet->data_size = data_len;
        packet->len = data_len;
        packet->data_owned = 0;
        packet->pos = 0;

        return packet;
}

void irmo_packet_free(IrmoPacket *packet)
{
        irmo_return_if_fail(packet != NULL);

        if (packet->data_owned) {
	        free(packet->data);
        }

	free(packet);
}

static void irmo_packet_resize(IrmoPacket *packet)
{
	// resize exponentially bigger

	packet->data_size *= 2;
	packet->data = irmo_realloc(packet->data, packet->data_size);
}

static void irmo_packet_update_len(IrmoPacket *packet)
{
	if (packet->pos > packet->len)
		packet->len = packet->pos;
}

int irmo_packet_writei8(IrmoPacket *packet, unsigned int i)
{
        irmo_return_val_if_fail(packet != NULL, 0);
        irmo_return_val_if_fail(packet->data_owned, 0);

	if (packet->pos + 1 > packet->data_size)
		irmo_packet_resize(packet);
	
	packet->data[packet->pos++] = (uint8_t) i;

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_writei16(IrmoPacket *packet, unsigned int i)
{
        irmo_return_val_if_fail(packet != NULL, 0);
        irmo_return_val_if_fail(packet->data_owned, 0);

	if (packet->pos + 2 > packet->data_size) {
		irmo_packet_resize(packet);
        }

	packet->data[packet->pos++] = (uint8_t) ((i >> 8) & 0xff);
	packet->data[packet->pos++] = (uint8_t) ((i) & 0xff);

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_writei32(IrmoPacket *packet, unsigned int i)
{
        irmo_return_val_if_fail(packet != NULL, 0);
        irmo_return_val_if_fail(packet->data_owned, 0);

	if (packet->pos + 4 > packet->data_size) {
		irmo_packet_resize(packet);
        }

	packet->data[packet->pos++] = (uint8_t) ((i >> 24) & 0xff);
	packet->data[packet->pos++] = (uint8_t) ((i >> 16) & 0xff);
	packet->data[packet->pos++] = (uint8_t) ((i >> 8) & 0xff);
	packet->data[packet->pos++] = (uint8_t) ((i) & 0xff);

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_writestring(IrmoPacket *packet, char *s)
{
        irmo_return_val_if_fail(packet != NULL, 0);
        irmo_return_val_if_fail(packet->data_owned, 0);

	if (packet->pos + strlen(s) + 1 > packet->data_size) {
		irmo_packet_resize(packet);
        }

	strcpy((char *) packet->data + packet->pos, s); 
	packet->pos += strlen(s) + 1;

	irmo_packet_update_len(packet);

	return 1;
}

int irmo_packet_readi8(IrmoPacket *packet, unsigned int *i)
{
        irmo_return_val_if_fail(packet != NULL, 0);

	if (packet->pos + 1 > packet->len) {
		return 0;
        }

	if (i != NULL) {
		*i = packet->data[packet->pos];
	}

	packet->pos += 1;

	return 1;
}

int irmo_packet_readi16(IrmoPacket *packet, unsigned int *i)
{
	uint8_t *data;
	
        irmo_return_val_if_fail(packet != NULL, 0);

	if (packet->pos + 2 > packet->len) {
		return 0;
        }

	data = packet->data + packet->pos;
	
	if (i != NULL) {
                *i = ((unsigned int) data[0]) << 8;
		*i |= (unsigned int) data[1];
	}

	packet->pos += 2;

	return 1;
}

int irmo_packet_readi32(IrmoPacket *packet, unsigned int *i)
{
	uint8_t *data;

        irmo_return_val_if_fail(packet != NULL, 0);

	if (packet->pos + 4 > packet->len) {
		return 0;
        }
	
	data = packet->data + packet->pos;

	if (i != NULL) {
		*i = ((unsigned int) data[0]) << 24;
                *i |= ((unsigned int) data[1]) << 16;
                *i |= ((unsigned int) data[2]) << 8;
                *i |= ((unsigned int) data[3]);
	}

	packet->pos += 4;

	return 1;
}

char *irmo_packet_readstring(IrmoPacket *packet)
{
	uint8_t *start = packet->data + packet->pos;

        irmo_return_val_if_fail(packet != NULL, NULL);

	for (; packet->pos < packet->len; ++packet->pos) {
		if (packet->data[packet->pos] == '\0') {
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
        irmo_return_val_if_fail(packet != NULL, 0);

	switch (type) {
	case IRMO_TYPE_INT8:
		return irmo_packet_readi8(packet, NULL);
	case IRMO_TYPE_INT16:
		return irmo_packet_readi16(packet, NULL);
	case IRMO_TYPE_INT32:
		return irmo_packet_readi32(packet, NULL);
	case IRMO_TYPE_STRING:
		return irmo_packet_readstring(packet) != NULL;
        default:
                irmo_bug();
                return 0;
	}
}

int irmo_packet_read_value(IrmoPacket *packet, IrmoValue *value, 
                           IrmoValueType type)
{
        char *strvalue;

        irmo_return_val_if_fail(packet != NULL, 0);

	switch (type) {
	case IRMO_TYPE_INT8:
		return irmo_packet_readi8(packet, &value->i);
	case IRMO_TYPE_INT16:
		return irmo_packet_readi16(packet, &value->i);
	case IRMO_TYPE_INT32:
		return irmo_packet_readi32(packet, &value->i);
	case IRMO_TYPE_STRING:
                strvalue = irmo_packet_readstring(packet);
                if (strvalue == NULL) {
                        return 0;
                } else {
                        value->s = strdup(strvalue);
                        return 1;
                }
        default:
                irmo_bug();
	}

        return 0;
}

void irmo_packet_write_value(IrmoPacket *packet, IrmoValue *value,
			     IrmoValueType type)
{
        irmo_return_if_fail(packet != NULL);

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
        default:
                irmo_bug();
	}
}

unsigned char *irmo_packet_get_buffer(IrmoPacket *packet)
{
        irmo_return_val_if_fail(packet != NULL, NULL);

        return packet->data;
}

unsigned int irmo_packet_get_length(IrmoPacket *packet)
{
        irmo_return_val_if_fail(packet != NULL, 0);

        return (unsigned int) packet->len;
}

unsigned int irmo_packet_get_position(IrmoPacket *packet)
{
        irmo_return_val_if_fail(packet != NULL, 0);

        return (unsigned int) packet->pos;
}

int irmo_packet_set_position(IrmoPacket *packet, unsigned int pos)
{
        irmo_return_val_if_fail(packet != NULL, 0);
        irmo_return_val_if_fail(pos <= packet->len, 0);

        packet->pos = pos;

        return 1;
}


