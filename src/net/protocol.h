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

#ifndef IRMO_NET_PROTO_H
#define IRMO_NET_PROTO_H

#include <irmo/packet.h>

#include "client.h"

// protocol version number, bumped every time the protocol changes

#define IRMO_PROTOCOL_VERSION 4

// Packet header flags

#define PACKET_FLAG_SYN 0x01
#define PACKET_FLAG_ACK 0x02
#define PACKET_FLAG_FIN 0x04
#define PACKET_FLAG_DTA 0x08

/*!
 * Verify that the specified packet is valid and can be parsed.
 *
 * @param packet        The packet to verify.
 * @param client        The client that the packet was received from.
 * @param flags         Header flags read from the packet.
 * @return              Non-zero if the packet was verified as valid.
 */

int irmo_proto_verify_packet(IrmoPacket *packet,
                             IrmoClient *client,
                             unsigned int flags);

/*!
 * Parse a packet received from a client.
 *
 * @param packet        The packet.
 * @param client        The client that the packet was receieved from.
 * @param flags         Header flags read from the packet.
 */

void irmo_proto_parse_packet(IrmoPacket *packet,
                             IrmoClient *client,
                             unsigned int flags);

/*!
 * Send new data to the specified client.  This is called periodically
 * for all clients to send updates.
 *
 * @param client        The client.
 */

void irmo_proto_run_client(IrmoClient *client);

#endif /* #ifndef IRMO_NET_PROTO_H */

