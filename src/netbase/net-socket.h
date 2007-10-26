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

#ifndef NETBASE_NET_SOCKET_H
#define NETBASE_NET_SOCKET_H

#include <irmo/net-module.h>

/*! 
 * Initialise a new client socket for use.
 *
 * @param module   Pointer to network module to use for communication.
 *
 * @return         Pointer to a new IrmoNetSocket, or NULL if unable
 *                 to open a socket.
 */

IrmoNetSocket *irmo_net_socket_open_unbound(IrmoNetModule *module);

/*!
 * Initialise a new server socket, bound to a port.
 *
 * @param module   Pointer to network module to use.
 * @param port     Numerical port to bind to.  Depending on the 
 *                 protocol, this may be ignored.
 * @return         Pointer to a new IrmoNetSocket, or NULL if unable
 *                 to open a socket.
 */

IrmoNetSocket *irmo_net_socket_open_bound(IrmoNetModule *module, int port);

/*!
 * Close a socket.
 *
 * @param sock     The socket to close.
 */

void irmo_net_socket_close(IrmoNetSocket *sock);

/*!
 * Transmit a packet to the specified address and port.
 *
 * @param sock     The socket to transmit on.
 * @param address  Address to transmit to.
 * @param packet   Packet to transmit.
 *
 * @return         Non-zero for success, zero for failure.
 */

int irmo_net_socket_send_packet(IrmoNetSocket *sock,
                                IrmoNetAddress *address,
                                IrmoPacket *packet);

/*!
 * Receive a packet.
 *
 * @param sock     Socket to receive on.
 * @param address  Pointer to a variable to save the source address.
 *
 * @return         A new IrmoPacket, or NULL if no packet was 
 *                 received.
 */

IrmoPacket *irmo_net_socket_recv_packet(IrmoNetSocket *sock,
                                        IrmoNetAddress **address);

/*!
 * Block on a list of sockets until a packet is received,
 * or the given timeout expires.
 *
 * @param sockets      Array of sockets to block on.
 * @param num_handles  Number of handles in the array.
 * @param timeout      Time to block for in ms, or 0 for infinite
 *                     timeout.
 */

int irmo_net_socket_block_set(IrmoNetSocket **sockets,
                              int num_handles,
                              int timeout);

/*!
 * Block on a single socket until a packet is received or the given
 * timeout expires.
 *
 * @param sock         The socket to block on.
 * @param timeout      Timeout to block for in ms, or 0 for infinite
 *                     timeout.
 */

int irmo_net_socket_block(IrmoNetSocket *sock, int timeout);

#endif /* #ifndef NETBASE_NET_SOCKET_H */

