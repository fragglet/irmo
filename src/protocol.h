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

#endif /* #ifndef IRMO_PROTO_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/03/07 12:31:51  sdh300
// Add protocol.h
//

