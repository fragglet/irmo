#ifndef IRMO_PROTO_H
#define IRMO_PROTO_H

#include "client.h"
#include "packet.h"

// parsing received packets

void proto_parse_packet(IrmoPacket *packet);

// running client to build and send packets

void proto_run_client(IrmoClient *client);

#endif /* #ifndef IRMO_PROTO_H */

// $Log: not supported by cvs2svn $

