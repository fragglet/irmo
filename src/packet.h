//
// Safe packet handling code
//

#ifndef IRMO_PACKET_H
#define IRMO_PACKET_H

#include <glib.h>
#include <netinet/in.h>

typedef struct _IrmoPacket IrmoPacket;

#include "socket.h"

#define	PACKET_FLAG_SYN 0x01
#define	PACKET_FLAG_ACK 0x02
#define	PACKET_FLAG_FIN 0x04
#define	PACKET_FLAG_NEW 0x08
#define	PACKET_FLAG_SET 0x10
#define	PACKET_FLAG_DEL 0x20
#define	PACKET_FLAG_FNC 0x40

struct _IrmoPacket {
	IrmoSocket *sock;       // socket this came from
	struct sockaddr *src;   // source address
	guchar *data;           // packet data
	gsize len;              // data length
	guint pos;              // current position in packet
	IrmoClient *client;     // client implied by address (or NULL if none)
	guint16 flags;          // flags from header
};

IrmoPacket *packet_new(gsize size);
void packet_free(IrmoPacket packet);

gboolean packet_writei8(IrmoPacket *packet, guchar c);
gboolean packet_writei16(IrmoPacket *packet, guint16 s);
gboolean packet_writei32(IrmoPacket *packet, guint32 l);

gboolean packet_readi8(IrmoPacket *packet, guchar *c);
gboolean packet_readi16(IrmoPacket *packet, guint16 *s);
gboolean packet_readi32(IrmoPacket *packet, guint32 *l);
gchar *packet_readstring(IrmoPacket *packet);

#endif /* #ifndef IRMO_PACKET_H */

// $Log: not supported by cvs2svn $
// Revision 1.4  2002/12/02 22:24:22  sdh300
// Initial socket run code/add extra data to packet objects
//
// Revision 1.3  2002/12/02 21:56:45  sdh300
// Fix build (compile errors)
//
// Revision 1.2  2002/12/02 21:34:50  sdh300
// Add CVS tags
//