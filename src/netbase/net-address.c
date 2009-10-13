//
// Copyright (C) 2002-2008 Simon Howard
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

#include <stdlib.h>

#include "net-address.h"

IrmoNetAddress *irmo_net_address_resolve(IrmoNetModule *module,
                                         char *address,
                                         unsigned int port)
{
        IrmoNetAddress *result;

        result = module->resolve_address(module, address, port);

        // Add a reference for this address.

        if (result != NULL) {

                irmo_net_address_ref(result);
        }

        return result;
}

void irmo_net_address_to_string(IrmoNetAddress *address,
                                char *buffer, unsigned int buffer_len)
{
        address->address_class->to_string(address, buffer, buffer_len);
}

unsigned int irmo_net_address_get_port(IrmoNetAddress *address)
{
        return address->address_class->get_port(address);
}

void irmo_net_address_ref(IrmoNetAddress *address)
{
        ++address->_refcount;
}

void irmo_net_address_unref(IrmoNetAddress *address)
{
        --address->_refcount;

        if (address->_refcount == 0) {
                address->address_class->free_address(address);
        }
}

