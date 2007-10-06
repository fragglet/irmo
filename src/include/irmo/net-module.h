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

/*!
 * A network module.
 */

typedef struct _IrmoNetModule IrmoNetModule;

struct _IrmoNetModule {

        /*! 
         * Initialise a new client socket for use.
         *
         * @param module   Pointer to this module.
         *
         * @return         Handle to use for accessing the module in the 
         *                 future, or NULL if unable to open a socket.
         */

        void *(*open_client_sock)(IrmoNetModule *module);

        /*!
         * Initialise a new server socket, bound to a port.
         *
         * @param module   Pointer to this module.
         * @param port     Numerical port to bind to.  Depending on the 
         *                 protocol, this may be ignored.
         * @return         Handle to use for accessing the module in the 
         *                 future, or NULL if unable to open a socket.
         */

        void *(*open_server_sock)(IrmoNetModule *module, int port);

        /*!
         * Close a socket.
         *
         * @param handle   Handle to the socket to close.
         */

        void (*close_sock)(void *handle);

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
         * @return         Handle to the address, or NULL if unable to
         *                 resolve the address. 
         */

        void *(*resolve_address)(IrmoNetModule *module,
                                 char *address,
                                 int port);

        /*!
         * Free an address.
         *
         * @param address  Handle to the address to free.
         */

        void (*free_address)(void *address);

        /*!
         * Transmit a packet to the specified address and port.
         *
         * @param handle   Handle to the socket to transmit on.
         * @param address  Address to transmit to.
         * @param packet   Packet to transmit.
         *
         * @return         Non-zero for success, zero for failure.
         */

        int (*send_packet)(void *handle, void *address,
                           IrmoPacket *packet);

        /*!
         * Receive a packet.
         *
         * @param handle   Socket to receive on.
         * @param address  Pointer to a variable to save the source address.
         *
         * @return         A new IrmoPacket, or NULL if no packet was 
         *                 received.
         */

        IrmoPacket *(*recv_packet)(void *handle, void **address);
};

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_NET_MODULE_H */

