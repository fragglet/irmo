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
// Network address abstraction layer.
//

#ifndef NETBASE_NET_ADDRESS_H
#define NETBASE_NET_ADDRESS_H

#include <irmo/net-module.h>

/*!
 * Resolve a string description of an address.
 * The returned address should be freed back using 
 * @ref irmo_net_address_free.
 *
 * @param module   Pointer to the network module to use.
 * @param address  Address to resolve.
 * @param port     Port at the remote address.  Depending on the
 *                 protocol, this may be ignored.
 *
 * @return         Pointer to an address, or NULL if unable to
 *                 resolve the address. 
 *
 * @sa irmo_net_address_free.
 */

IrmoNetAddress *irmo_net_address_resolve(IrmoNetModule *module,
                                         char *address,
                                         int port);

/*!
 * Format an address in a user-presentable format.
 *
 * @param address  The address.
 * @return         Pointer to a static buffer containing a text
 *                 description of the address.
 */

char *irmo_net_address_to_string(IrmoNetAddress *address);

/*!
 * Add a reference to an address.
 *
 * @param address          Address to reference.
 */

void irmo_net_address_ref(IrmoNetAddress *address);

/*!
 * Remove a reference to an address.
 *
 * @param address          Address to unreference.
 */

void irmo_net_address_unref(IrmoNetAddress *address);

#endif /* #ifndef NETBASE_NET_ADDRESS_H */

