//
// Safe packet handling code
//

#ifndef IRMO_PACKET_H
#define IRMO_PACKET_H

#include <glib.h>

typedef struct _IrmoPacket IrmoPacket;

struct _IrmoPacket {
	guchar *data;
	gsize len;              // data length
	guint pos;              // current position in packet
};

gboolean packet_readi8(IrmoPacket *packet, guchar *c);
gboolean packet_readi16(IrmoPacket *packet, guint16 *s);
gboolean packet_readi32(IrmoPacket *packet, guint32 *l);

#endif /* #ifndef IRMO_PACKET_H */

// $Log: not supported by cvs2svn $
// Revision 1.2  2002/12/02 21:34:50  sdh300
// Add CVS tags
//
