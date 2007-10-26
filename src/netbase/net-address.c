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

#include "net-address.h"

IrmoNetAddress *irmo_net_address_resolve(IrmoNetModule *module,
                                         char *address,
                                         int port)
{
        IrmoNetAddress *result;

        result = module->resolve_address(module, address, port);

        // First time this has been resolved?  Start with a refcount of 1.

        if (result->_refcount == 0) {
                result->_refcount = 1;
        }

        return result;
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

