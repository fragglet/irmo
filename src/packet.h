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

#ifndef IRMO_PACKET_H
#define IRMO_PACKET_H

#include <glib.h>
#include <netinet/in.h>

typedef struct _IrmoPacket IrmoPacket;

#include "object.h"
#include "socket.h"

#define	PACKET_FLAG_SYN 0x01
#define	PACKET_FLAG_ACK 0x02
#define	PACKET_FLAG_FIN 0x04
#define PACKET_FLAG_DTA 0x08

struct _IrmoPacket {
	IrmoSocket *sock;       // socket this came from
	struct sockaddr *src;   // source address
	guchar *data;           // packet data
	gsize data_size;	// size of the buffer
	gsize len;              // length of used data in the buffer
	guint pos;              // current position in packet
	IrmoClient *client;     // client implied by address (or NULL if none)
	guint flags;            // flags from header
};

IrmoPacket *irmo_packet_new(void);
void irmo_packet_free(IrmoPacket *packet);

gboolean irmo_packet_writei8(IrmoPacket *packet, guint i);
gboolean irmo_packet_writei16(IrmoPacket *packet, guint i);
gboolean irmo_packet_writei32(IrmoPacket *packet, guint i);
gboolean irmo_packet_writestring(IrmoPacket *packet, gchar *s);

gboolean irmo_packet_readi8(IrmoPacket *packet, guint *i);
gboolean irmo_packet_readi16(IrmoPacket *packet, guint *i);
gboolean irmo_packet_readi32(IrmoPacket *packet, guint *i);
gchar *irmo_packet_readstring(IrmoPacket *packet);

gboolean irmo_packet_verify_value(IrmoPacket *packet, IrmoValueType type);
void irmo_packet_read_value(IrmoPacket *packet, IrmoValue *value, 
			    IrmoValueType type);
void irmo_packet_write_value(IrmoPacket *packet, IrmoValue *value, 
			     IrmoValueType type);

#endif /* #ifndef IRMO_PACKET_H */

// $Log$
// Revision 1.5  2003/10/22 16:05:01  fraggle
// Move field reading routines into packet.c
//
// Revision 1.4  2003/10/14 22:12:49  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.3  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.2  2003/09/01 01:25:49  fraggle
// Improve packet code; increase packet size exponentially.
// Remove the need to specify the size when creating a new packet object.
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.10  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.9  2003/03/06 20:15:25  sdh300
// Initial ack code
//
// Revision 1.8  2003/03/03 22:22:30  sdh300
// Change header flag defines as I changed my plans for the protocol
//
// Revision 1.7  2003/03/02 02:11:30  sdh300
// Add missing packet_writestring prototype
//
// Revision 1.6  2003/02/03 21:12:31  sdh300
// Fix errors
//
// Revision 1.5  2003/02/03 20:45:27  sdh300
// add packet string code
// add packet writing code
// add packet flags
//
// Revision 1.4  2002/12/02 22:24:22  sdh300
// Initial socket run code/add extra data to packet objects
//
// Revision 1.3  2002/12/02 21:56:45  sdh300
// Fix build (compile errors)
//
// Revision 1.2  2002/12/02 21:34:50  sdh300
// Add CVS tags
//
