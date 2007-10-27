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

#ifndef IRMO_NET_MODULE_H
#define IRMO_NET_MODULE_H

#include "types.h"
#include "packet.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * Network modules.  Low level sending/receiving of packets is abstracted
 * below this interface.  By writing new modules, new transport mediums
 * can be supported.
 *
 * @addtogroup net_module
 * \{
 */

typedef struct _IrmoNetAddressClass IrmoNetAddressClass;
typedef struct _IrmoNetSocketClass IrmoNetSocketClass;
typedef struct _IrmoNetSocket IrmoNetSocket;

/*!
 * A class of network address.  Every network address has a class
 * associated with it.
 */

struct _IrmoNetAddressClass {

        /*!
         * Format the address as a presentable text string.
         *
         * @param address       The address.
         * @return              Pointer to a static buffer containing
         *                      a text description of the address.
         */

        char *(*to_string)(IrmoNetAddress *address);

        /*!
         * Free an address.
         *
         * @param address       The address to free.
         */

        void (*free_address)(IrmoNetAddress *address);
};

/*!
 * A remote network address.
 */

struct _IrmoNetAddress {
        IrmoNetAddressClass *address_class;
        int _refcount;
};

/*!
 * A class of network socket.  Every network socket has a class 
 * associated with it.
 */

struct _IrmoNetSocketClass {

        /*!
         * Close the socket.
         *
         * @param sock     The socket to close.
         */

        void (*close)(IrmoNetSocket *sock);

        /*!
         * Transmit a packet to the specified address and port.
         *
         * @param sock     The socket to transmit on.
         * @param address  Address to transmit to.
         * @param packet   Packet to transmit.
         *
         * @return         Non-zero for success, zero for failure.
         */

        int (*send_packet)(IrmoNetSocket *sock,
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

        IrmoPacket *(*recv_packet)(IrmoNetSocket *sock,
                                   IrmoNetAddress **address);

        /*!
         * Block on a list of sockets until a packet is received,
         * or the given timeout expires.
         *
         * @param sockets      Array of sockets to block on.
         * @param num_sockets  Number of sockets in the array.
         * @param timeout      Time to block for in ms, or 0 for infinite
         *                     timeout.
         */

        int (*block_set)(IrmoNetSocket **sockets,
                         int num_sockets,
                         int timeout);
};

/*!
 * An open network socket, used for sending and receiving packets.
 */

struct _IrmoNetSocket {
        IrmoNetSocketClass *socket_class;
};

/*!
 * A network module, defining callback functions to use for network
 * communications.
 */

struct _IrmoNetModule {

        /*! 
         * Initialise a new client socket for use.
         *
         * @param module   Pointer to this module.
         *
         * @return         A new @ref IrmoNetSocket, or NULL if it was 
         *                 not possible to open the socket.
         */

        IrmoNetSocket *(*open_client_sock)(IrmoNetModule *module);

        /*!
         * Initialise a new server socket, bound to a port.
         *
         * @param module   Pointer to this module.
         * @param port     Numerical port to bind to.  Depending on the 
         *                 protocol, this may be ignored.
         * @return         A new @ref IrmoNetSocket, or NULL if it was 
         *                 not possible to open the socket.
         */

        IrmoNetSocket *(*open_server_sock)(IrmoNetModule *module, int port);

        /*!
         * Resolve a string description of an address.
         * The returned address should be freed back using 
         * @ref free_address (below).
         *
         * @param module   Pointer to this module.
         * @param address  Address to resolve.
         * @param port     Port at the remote address.  Depending on the
         *                 protocol, this may be ignored.
         *
         * @return         Pointer to an address, or NULL if unable to
         *                 resolve the address. 
         */

        IrmoNetAddress *(*resolve_address)(IrmoNetModule *module,
                                           char *address,
                                           int port);
};

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_NET_MODULE_H */

