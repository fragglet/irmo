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

//
// Safe packet handling code
//

#include <stdlib.h>
#include <string.h>

#include "packet.h"

IrmoPacket *irmo_packet_new(void)
{
	IrmoPacket *packet = g_new0(IrmoPacket, 1);

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

G_INLINE_FUNC void irmo_packet_resize(IrmoPacket *packet)
{
	// resize exponentially bigger

	packet->data_size *= 2;
	packet->data = realloc(packet->data, packet->data_size);
}

G_INLINE_FUNC void irmo_packet_update_len(IrmoPacket *packet)
{
	if (packet->pos > packet->len)
		packet->len = packet->pos;
}

gboolean irmo_packet_writei8(IrmoPacket *packet, guint i)
{
	if (packet->pos + 1 > packet->data_size)
		irmo_packet_resize(packet);
	
	packet->data[packet->pos++] = i;

	irmo_packet_update_len(packet);

	return TRUE;
}

gboolean irmo_packet_writei16(IrmoPacket *packet, guint i)
{
	if (packet->pos + 2 > packet->data_size)
		irmo_packet_resize(packet);

	packet->data[packet->pos++] = (i >> 8) & 0xff;
	packet->data[packet->pos++] = (i) & 0xff;

	irmo_packet_update_len(packet);

	return TRUE;
}

gboolean irmo_packet_writei32(IrmoPacket *packet, guint i)
{
	if (packet->pos + 4 > packet->data_size)
		irmo_packet_resize(packet);

	packet->data[packet->pos++] = (i >> 24) & 0xff;
	packet->data[packet->pos++] = (i >> 16) & 0xff;
	packet->data[packet->pos++] = (i >> 8) & 0xff;
	packet->data[packet->pos++] = (i) & 0xff;

	irmo_packet_update_len(packet);

	return TRUE;
}

gboolean irmo_packet_writestring(IrmoPacket *packet, gchar *s)
{
	if (packet->pos + strlen(s) + 1 > packet->data_size)
		irmo_packet_resize(packet);

	strcpy(packet->data + packet->pos, s);
	packet->pos += strlen(s) + 1;

	irmo_packet_update_len(packet);

	return TRUE;
}

gboolean irmo_packet_readi8(IrmoPacket *packet, guint *i)
{
	if (packet->pos + 1 > packet->len)
		return FALSE;

	if (i) {
		*i = packet->data[packet->pos];
	}

	packet->pos += 1;

	return TRUE;
}

gboolean irmo_packet_readi16(IrmoPacket *packet, guint *i)
{
	guchar *data;
	
	if (packet->pos + 2 > packet->len)
		return FALSE;

	data = packet->data + packet->pos;
	
	if (i) {
		*i = (data[0] << 8) + data[1];
	}

	packet->pos += 2;

	return TRUE;
}

gboolean irmo_packet_readi32(IrmoPacket *packet, guint *i)
{
	guchar *data;

	if (packet->pos + 4 > packet->len)
		return FALSE;
		
	data = packet->data + packet->pos;

	if (i) {
		*i = (data[0] << 24) + (data[1] << 16) 
		   + (data[2] << 8) + data[3];
	}

	packet->pos += 4;

	return TRUE;
}

gchar *irmo_packet_readstring(IrmoPacket *packet)
{
	guchar *start = packet->data + packet->pos;

	for (; packet->pos < packet->len; ++packet->pos) {
		if (!packet->data[packet->pos]) {
			// skip past the terminating 0
			
			++packet->pos;
			
			return start;
		}
	}

	// overflowed past the end of the packet
	
	return NULL;
}

gboolean irmo_packet_verify_value(IrmoPacket *packet,
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

// $Log$
// Revision 1.6  2003/10/22 16:05:01  fraggle
// Move field reading routines into packet.c
//
// Revision 1.5  2003/10/14 22:12:49  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.4  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.3  2003/09/01 01:25:49  fraggle
// Improve packet code; increase packet size exponentially.
// Remove the need to specify the size when creating a new packet object.
//
// Revision 1.2  2003/08/18 01:23:14  fraggle
// Use G_INLINE_FUNC instead of inline for portable inline function support
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.9  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.8  2003/02/18 20:04:40  sdh300
// Automatically increase size of packets when writing
//
// Revision 1.7  2003/02/11 19:10:03  sdh300
// fix packet_writestring
//
// Revision 1.6  2003/02/11 19:04:07  sdh300
// Fix another bug in packet_readstring
//
// Revision 1.5  2003/02/11 19:02:03  sdh300
// fix packet_readstring bug
//
// Revision 1.4  2003/02/03 21:12:31  sdh300
// Fix errors
//
// Revision 1.3  2003/02/03 20:45:27  sdh300
// add packet string code
// add packet writing code
// add packet flags
//
// Revision 1.2  2002/12/02 21:34:49  sdh300
// Add CVS tags
//
