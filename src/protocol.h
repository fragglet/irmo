// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id: protocol.h,v 1.4 2003-06-09 21:06:52 sdh300 Exp $
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

// verifying integrity of received packets before parsing

gboolean proto_verify_packet(IrmoPacket *packet);

// parsing received packets

void proto_parse_packet(IrmoPacket *packet);

// running client to build and send packets

void proto_run_client(IrmoClient *client);

// timeout time for a particular client

int proto_client_timeout_time(IrmoClient *client);

#endif /* #ifndef IRMO_PROTO_H */

// $Log: not supported by cvs2svn $
// Revision 1.3  2003/03/21 17:21:46  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.2  2003/03/14 01:07:23  sdh300
// Initial packet verification code
//
// Revision 1.1  2003/03/07 12:31:51  sdh300
// Add protocol.h
//

