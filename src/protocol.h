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

#ifndef IRMO_PROTO_H
#define IRMO_PROTO_H

#include "client.h"
#include "packet.h"

// protocol version number, bumped every time the protocol changes

#define IRMO_PROTOCOL_VERSION 3

// verifying integrity of received packets before parsing

gboolean irmo_proto_verify_packet(IrmoPacket *packet);

// parsing received packets

void irmo_proto_parse_packet(IrmoPacket *packet);

// running client to build and send packets

void irmo_proto_run_client(IrmoClient *client);

// timeout time for a particular client

int irmo_proto_client_timeout_time(IrmoClient *client);

#endif /* #ifndef IRMO_PROTO_H */

// $Log$
// Revision 1.4  2003/10/18 01:34:45  fraggle
// Better error reporting for connecting, allow server to send back an
// error message when refusing connections
//
// Revision 1.3  2003/10/17 23:33:05  fraggle
// protocol version checking
//
// Revision 1.2  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.4  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.3  2003/03/21 17:21:46  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.2  2003/03/14 01:07:23  sdh300
// Initial packet verification code
//
// Revision 1.1  2003/03/07 12:31:51  sdh300
// Add protocol.h
//

