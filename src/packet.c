//
// Safe packet handling code
//

#include "packet.h"

IrmoPacket *packet_new(gsize size)
{
	IrmoPacket *packet = g_new0(IrmoPacket, 1);

	packet->data = malloc(size);
	packet->size = size;
	packet->pos = 0;

	return packet;
}

void packet_free(IrmoPacket *packet)
{
	free(packet->data);
	free(packet->src);
	free(packet);
}

gboolean packet_writei8(IrmoPacket *packet, guchar c)
{
	if (packet->pos + 1 > packet->len)
		return FALSE;

	packet->data[packet->pos++] = c;

	return TRUE;
}

gboolean packet_writei16(IrmoPacket *packet, guint16 s)
{
	if (packet->pos + 2 > packet->len)
		return FALSE;

	packet->data[packet->pos++] = (s >> 8) & 0xff;
	packet->data[packet->pos++] = (s) & 0xff;

	return TRUE;
}

gboolean packet_writei32(IrmoPacket *packet, guint32 l)
{
	if (packet->pos + 4 > packet->len)
		return FALSE;

	packet->data[packet->pos++] = (s >> 24) & 0xff;
	packet->data[packet->pos++] = (s >> 16) & 0xff;
	packet->data[packet->pos++] = (s >> 8) & 0xff;
	packet->data[packet->pos++] = (s) & 0xff;

	return TRUE;
}

gboolean packet_writestring(IrmoPacket *packet, gchar *s)
{
	if (packet->pos + strlen(s) + 1 > packet->len)
		return FALSE;

	strcpy(packet->data + packet->pos, s);
	packet->pos += strlen(s);

	return TRUE;
}

gboolean packet_readi8(IrmoPacket *packet, guchar *c)
{
	if (packet->pos + 1 > packet->len)
		return FALSE;

	*c = packet->data[packet->pos++];

	return TRUE;
}

gboolean packet_readi16(IrmoPacket *packet, guint16 *s)
{
	guchar *data;
	
	if (packet->pos + 2 > packet->len)
		return FALSE;

	data = packet->data + packet->pos;
	
	*s = (data[0] << 8) + data[1];
	
	packet->pos += 2;

	return TRUE;
}

gboolean packet_readi32(IrmoPacket *packet, guint32 *l)
{
	guchar *data;

	if (packet->pos + 4 > packet->len)
		return FALSE;
		
	data = packet->data + packet->pos;

	*l = (data[0] << 24) + (data[1] << 16) + (data[2] << 8) + data[3];

	packet->pos += 4;

	return TRUE;
}

gchar *packet_readstring(IrmoPacket *packet)
{
	guchar *start = packet->data + packet->pos;

	while (packet->pos < packet->len) {
		if (!packet->data[packet->pos])
			return start;
	}

	// overflowed past the end of the packet
	
	return NULL;
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2002/12/02 21:34:49  sdh300
// Add CVS tags
//
