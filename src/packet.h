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

//
// Safe packet handling code
//

#ifndef IRMO_PACKET_H
#define IRMO_PACKET_H

typedef struct _IrmoPacket IrmoPacket;

#include "netlib.h"
#include "object.h"
#include "socket.h"

#define	PACKET_FLAG_SYN 0x01
#define	PACKET_FLAG_ACK 0x02
#define	PACKET_FLAG_FIN 0x04
#define PACKET_FLAG_DTA 0x08

struct _IrmoPacket {
	IrmoSocket *sock;       // socket this came from
	struct sockaddr *src;   // source address
	uint8_t *data;          // packet data
	size_t data_size;       // size of the buffer
	size_t len;             // length of used data in the buffer
	unsigned int pos;       // current position in packet
	IrmoClient *client;     // client implied by address (or NULL if none)
	unsigned int flags;     // flags from header
};

IrmoPacket *irmo_packet_new(void);
void irmo_packet_free(IrmoPacket *packet);

int irmo_packet_writei8(IrmoPacket *packet, unsigned int i);
int irmo_packet_writei16(IrmoPacket *packet, unsigned int i);
int irmo_packet_writei32(IrmoPacket *packet, unsigned int i);
int irmo_packet_writestring(IrmoPacket *packet, char *s);

int irmo_packet_readi8(IrmoPacket *packet, unsigned int *i);
int irmo_packet_readi16(IrmoPacket *packet, unsigned int *i);
int irmo_packet_readi32(IrmoPacket *packet, unsigned int *i);
char *irmo_packet_readstring(IrmoPacket *packet);

int irmo_packet_verify_value(IrmoPacket *packet, IrmoValueType type);
void irmo_packet_read_value(IrmoPacket *packet, IrmoValue *value, 
			    IrmoValueType type);
void irmo_packet_write_value(IrmoPacket *packet, IrmoValue *value, 
			     IrmoValueType type);

#endif /* #ifndef IRMO_PACKET_H */

