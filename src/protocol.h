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

#ifndef IRMO_PROTO_H
#define IRMO_PROTO_H

#include "client.h"
#include "packet.h"

// protocol version number, bumped every time the protocol changes

#define IRMO_PROTOCOL_VERSION 4

// verifying integrity of received packets before parsing

int irmo_proto_verify_packet(IrmoPacket *packet);

// parsing received packets

void irmo_proto_parse_packet(IrmoPacket *packet);

// running client to build and send packets

void irmo_proto_run_client(IrmoClient *client);

// timeout time for a particular client

int irmo_proto_client_timeout_time(IrmoClient *client);

#endif /* #ifndef IRMO_PROTO_H */

