//
// Copyright (C) 2007 Simon Howard
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
// Network socket abstraction layer.
//

#include "net-socket.h"

IrmoNetSocket *irmo_net_socket_open_unbound(IrmoNetModule *module)
{
        return module->open_client_sock(module);
}

IrmoNetSocket *irmo_net_socket_open_bound(IrmoNetModule *module, int port)
{
        return module->open_server_sock(module, port);
}

void irmo_net_socket_close(IrmoNetSocket *sock)
{
        sock->socket_class->close(sock);
}

int irmo_net_socket_send_packet(IrmoNetSocket *sock,
                                IrmoNetAddress *address,
                                IrmoPacket *packet)
{
        return sock->socket_class->send_packet(sock, address, packet);
}

IrmoPacket *irmo_net_socket_recv_packet(IrmoNetSocket *sock,
                                        IrmoNetAddress **address)
{
        return sock->socket_class->recv_packet(sock, address);
}

int irmo_net_socket_block_set(IrmoNetSocket **sockets,
                              int num_sockets,
                              int timeout)
{
        int (*block_set)(IrmoNetSocket **sockets, int nh, int to);
        int same_block_handler;
        int i;
       
        /* Check if all sockets have the same block_set handler. */

        block_set = sockets[0]->socket_class->block_set;
        same_block_handler = 1;

        for (i=1; i<num_sockets; ++i) {
                if (sockets[i]->socket_class->block_set != block_set) {
                        same_block_handler = 0;
                        break;
                }
        }

        /* If they all have the same handler, call it.  Otherwise, just
           block on the first socket.  Use a short timeout, as we will
           have to poll all the sockets. */

        if (same_block_handler) {
                return block_set(sockets, num_sockets, timeout);
        } else {
                return block_set(sockets, 1, 1);
        }
}

int irmo_net_socket_block(IrmoNetSocket *sock, int timeout)
{
        return sock->socket_class->block_set(&sock, 1, timeout);
}

