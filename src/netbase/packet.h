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

#ifndef IRMO_NETBASE_PACKET_H
#define IRMO_NETBASE_PACKET_H

#include <irmo/packet.h>

struct _IrmoPacket {
        // Packet data
	uint8_t *data;

        // Size of the buffer (malloced)
        // If data_size < 0, the packet does not own the buffer,
        // and should not free the buffer when freed.
	size_t data_size;

        // Length of used data in the buffer
	size_t len;

        // Current position in packet
	unsigned int pos;
};

#endif /* #ifndef IRMO_NETBASE_PACKET_H */

