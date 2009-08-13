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
#include "base/alloc.h"

#include "interface/interface.h"
#include "world/object.h"

#include <irmo/iterator.h>

#include "sendatom.h"

IrmoSendAtom *irmo_client_sendq_pop(IrmoClient *client)
{
	IrmoSendAtom *atom;

	while (1) {
	
		atom = (IrmoSendAtom *) irmo_queue_pop_head(client->sendq);
		
		if (atom == NULL) {
			return NULL;
                }

		// automatically ignore and delete NULL atoms that
		// are in the sendq
		
		if (atom->klass != &irmo_null_atom) {
			break;
                }
		
		irmo_sendatom_free(atom);
	} 

	// if a change, remove from the change hash

	if (atom->klass == &irmo_change_atom) {
		IrmoChangeAtom *catom = (IrmoChangeAtom *) atom;

		irmo_hash_table_remove(client->sendq_hashtable,
				       IRMO_POINTER_KEY(catom->object->id));
	}

	return atom;
}

void irmo_client_sendq_push(IrmoClient *client, IrmoSendAtom *atom)
{
	if (atom->klass == &irmo_change_atom) {
		IrmoChangeAtom *catom = (IrmoChangeAtom *) atom;

		irmo_hash_table_insert(client->sendq_hashtable,
				       IRMO_POINTER_KEY(catom->object->id),
				       atom);
	}
	
	atom->client = client;
	atom->len = atom->klass->length(atom);
	
	irmo_queue_push_tail(client->sendq, atom);
}

void irmo_client_sendq_add_new(IrmoClient *client, IrmoObject *object)
{
	IrmoNewObjectAtom *atom;

	atom = irmo_new0(IrmoNewObjectAtom, 1);

	atom->sendatom.klass = &irmo_newobject_atom;
	atom->id = object->id;
	atom->classnum = object->objclass->index;

	irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
}

static void clear_existing_change(IrmoClient *client,
                                  IrmoObject *obj,
                                  IrmoClassVar *var)
{
	IrmoChangeAtom *atom;
	unsigned int i;

	// Search the send window for a change atom affecting this
        // object.  If the atom changes this variable, that change
        // is out of date and should be removed - it does not need
        // to be retransmitted.

        // Once a change atom has no variables left to change, it
        // is nullified (converted to a null atom).

	for (i=0; i<client->sendwindow_size; ++i) {

		// is this a change atom for this variable in
		// this object?

		if (client->sendwindow[i]->klass != &irmo_change_atom) {
			continue;
                }

		atom = (IrmoChangeAtom *) client->sendwindow[i];

		if (atom->object == obj && atom->changed[var->index]) {

			// Unset the change in the atom and update
			// change count

			atom->changed[var->index] = 0;
			--atom->nchanged;

			// If there are no more changes, replace the atom
			// with a NULL

			if (atom->nchanged <= 0) {
				irmo_sendatom_nullify(IRMO_SENDATOM(atom));
			}

			// There can only be one change atom for a
			// variable in the send window, so stop.

			break;
		}
	}
}

void irmo_client_sendq_add_change(IrmoClient *client,
				  IrmoObject *object,
                                  IrmoClassVar *var)
{
	IrmoChangeAtom *atom;

        // Clear out an existing change for this variable, if one
        // is already in the send window - that change is now out
        // of date.
        // Don't nullify atoms until the world state has been
        // synchronized, as the client needs to have the complete
        // world state when it reaches the synchronization point.

        if (client->remote_synced) {
                clear_existing_change(client, object, var);
        }

	// Check if there is an existing atom for this object in
	// the send queue, and reuse it if possible.

	atom = irmo_hash_table_lookup(client->sendq_hashtable,
				      IRMO_POINTER_KEY(object->id));

	if (atom == NULL) {
		atom = irmo_new0(IrmoChangeAtom, 1);
		IRMO_SENDATOM(atom)->klass = &irmo_change_atom;

		atom->id = object->id;
		atom->object = object;
		atom->changed = irmo_new0(int, object->objclass->nvariables);
		atom->nchanged = 0;

		irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
	}

	// Set the change in the atom and update the change count

	if (!atom->changed[var->index]) {
		atom->changed[var->index] = 1;
		++atom->nchanged;
	}

	// Recalculate atom size

	atom->sendatom.len = irmo_change_atom.length(IRMO_SENDATOM(atom));
}

void irmo_client_sendq_add_destroy(IrmoClient *client, IrmoObject *object)
{
	IrmoDestroyAtom *atom;
	unsigned int i;
	
	// Check for any change atoms referring to this object

	atom = irmo_hash_table_lookup(client->sendq_hashtable,
				      IRMO_POINTER_KEY(object->id));

	// Convert to a ATOM_NULL atom

	if (atom != NULL) {
		irmo_sendatom_nullify(IRMO_SENDATOM(atom));
		irmo_hash_table_remove(client->sendq_hashtable,
				       IRMO_POINTER_KEY(object->id));
	}

	// Nullify atoms in send window too

	for (i=0; i<client->sendwindow_size; ++i) {
		IrmoChangeAtom *catom;

		if (client->sendwindow[i]->klass != &irmo_change_atom) {
			continue;
                }

		catom = (IrmoChangeAtom *) client->sendwindow[i];

		if (object == catom->object) {
			irmo_sendatom_nullify(client->sendwindow[i]);
		}
	}

	// create a destroy atom

	atom = irmo_new0(IrmoDestroyAtom, 1);
	atom->sendatom.klass = &irmo_destroy_atom;

	atom->id = object->id;

	irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
}

void irmo_client_sendq_add_method(IrmoClient *client, IrmoMethodData *data)
{
	IrmoMethod *method = data->method;
	IrmoMethodAtom *atom;
	unsigned int i;

	// create a new method atom

	atom = irmo_new0(IrmoMethodAtom, 1);
	atom->sendatom.klass = &irmo_method_atom;
	atom->method_data.method = method;

	// copy arguments

	atom->method_data.args = irmo_new0(IrmoValue, method->narguments);
	memcpy(atom->method_data.args,
	       data->args,
	       sizeof(IrmoValue) * method->narguments);

	// duplicate all the strings
	
	for (i=0; i<method->narguments; ++i) {
		if (method->arguments[i]->type == IRMO_TYPE_STRING) {
			atom->method_data.args[i].s
				= strdup(atom->method_data.args[i].s);
                }
	}

	// add to queue

	irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
}

// send a send window upper limit

void irmo_client_sendq_add_sendwindow(IrmoClient *client, unsigned int max)
{
	IrmoSendWindowAtom *atom;

	atom = irmo_new0(IrmoSendWindowAtom, 1);
	atom->sendatom.klass = &irmo_sendwindow_atom;

	atom->max = max;

	irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
}

// Add a synchronization point atom.

void irmo_client_sendq_add_sync_point(IrmoClient *client)
{
        IrmoSendAtom *atom;

	atom = irmo_new0(IrmoSendAtom, 1);
	atom->klass = &irmo_sync_point_atom;

	irmo_client_sendq_push(client, atom);

        // The client is no longer synced.  This will be reset
        // once the synchronization point atom has been acknowledged
        // received by the client.

        client->remote_synced = 0;

}

// queue up the entire current world state in the client send queue
// this is used for when new clients connect to retrieve the entire
// current world state

void irmo_client_sendq_add_state(IrmoClient *client)
{
        IrmoIterator *iter;
        IrmoClassVar *var;
        IrmoObject *object;
        unsigned int i;

	// create all the objects first
	// this is done all together and separately from sending the
	// variable state, as the rle encoding in the packets will
	// better compress the atoms this way

        iter = irmo_world_iterate_objects(client->server->world, NULL);

        while (irmo_iterator_has_more(iter)) {
                object = irmo_iterator_next(iter);

                irmo_client_sendq_add_new(client, object);
        }

        irmo_iterator_free(iter);

        // Now set the variable states

        iter = irmo_world_iterate_objects(client->server->world, NULL);

        while (irmo_iterator_has_more(iter)) {
                object = irmo_iterator_next(iter);

                // Add change for all variables in this object.

                for (i=0; i<object->objclass->nvariables; ++i) {
                        var = object->objclass->variables[i];
                        irmo_client_sendq_add_change(client, object, var);
                }
        }

        irmo_iterator_free(iter);

        // Send sync point to terminate initial state

        irmo_client_sendq_add_sync_point(client);
}

