//
// Safe packet handling code
//

#include "packet.h"

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

// $Log: not supported by cvs2svn $
