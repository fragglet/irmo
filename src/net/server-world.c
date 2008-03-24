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

#include "arch/sysheaders.h"
#include "sendatom.h"
#include "server-world.h"

// Called when the world being served creates a new object.

void irmo_server_object_new(IrmoServer *server, IrmoObject *obj)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {
                client = irmo_hash_table_iter_next(iter);

                // Add a new send atom for this object creation.
 
                irmo_client_sendq_add_new(client, obj);
        }

        irmo_hash_table_iter_free(iter);
}

// Called when the world being served destroys an object.

void irmo_server_object_destroyed(IrmoServer *server, IrmoObject *obj)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {
                client = irmo_hash_table_iter_next(iter);

                // Add a new send atom for this object destroy.
  
                irmo_client_sendq_add_destroy(client, obj);
        }

        irmo_hash_table_iter_free(iter);
}

// Called when the world being served changes an object.

void irmo_server_object_changed(IrmoServer *server, IrmoObject *obj,
                                IrmoClassVar *var)
{
        IrmoHashTableIterator *iter;
        IrmoClient *client;

        iter = irmo_hash_table_iterate(server->clients);

        while (irmo_hash_table_iter_has_more(iter)) {
                client = irmo_hash_table_iter_next(iter);

                // Add a new sendatom for this change.

                irmo_client_sendq_add_change(client, obj, var);
        }

        irmo_hash_table_iter_free(iter);
}

void irmo_connection_method_call(IrmoConnection *conn, 
                                 IrmoMethodData *data)
{
        // Add a new sendatom for this method call.

        irmo_client_sendq_add_method(conn, data);
}

