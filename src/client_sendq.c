// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
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
//---------------------------------------------------------------------

#include "sysheaders.h"

#include "if_spec.h"
#include "netlib.h"
#include "object.h"
#include "sendatom.h"

IrmoSendAtom *irmo_client_sendq_pop(IrmoClient *client)
{
	IrmoSendAtom *atom;

	while (1) {
	
		atom = (IrmoSendAtom *) irmo_queue_pop_head(client->sendq);
		
		if (!atom)
			return NULL;

		// automatically ignore and delete NULL atoms that
		// are in the sendq
		
		if (atom->klass != &irmo_null_atom)
			break;
		
		irmo_sendatom_free(atom);
	} 

	// if a change, remove from the change hash

	if (atom->klass == &irmo_change_atom) {
		IrmoChangeAtom *catom = (IrmoChangeAtom *) atom;

		irmo_hash_table_remove(client->sendq_hashtable,
				    (void *) catom->object->id);
	}

	return atom;
}

void irmo_client_sendq_push(IrmoClient *client, IrmoSendAtom *atom)
{
	if (atom->klass == &irmo_change_atom) {
		IrmoChangeAtom *catom = (IrmoChangeAtom *) atom;

		irmo_hash_table_insert(client->sendq_hashtable,
				    (void *) catom->object->id,
				    atom);
	}
	
	atom->client = client;
	atom->len = atom->klass->length(atom);
	
	irmo_queue_push_tail(client->sendq, atom);
}

void irmo_sendatom_free(IrmoSendAtom *atom)
{
	if (atom->klass->destructor)
		atom->klass->destructor(atom);

	free(atom);
}

static void irmo_sendatom_nullify(IrmoSendAtom *atom)
{
	if (atom->klass->destructor)
		atom->klass->destructor(atom);

	atom->klass = &irmo_null_atom;
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

void irmo_client_sendq_add_change(IrmoClient *client,
				  IrmoObject *object, int variable)
{
	IrmoChangeAtom *atom;
	int i;
	
	// search the send window and nullify this variable if there
	// is an existing change for it waiting to be acked

	for (i=0; i<client->sendwindow_size; ++i) {
		// check this is a change atom for this variable in
		// this object
		
		if (client->sendwindow[i]->klass != &irmo_change_atom)
			continue;

		atom = (IrmoChangeAtom *) client->sendwindow[i];

		if (atom->object == object && atom->changed[variable]) {

			// unset the change in the atom. update
			// change count
			
			atom->changed[variable] = 0;
			--atom->nchanged;

			// if there are no more changes, replace the atom
			// with a NULL

			if (atom->nchanged <= 0) {
				irmo_sendatom_nullify(IRMO_SENDATOM(atom));
			}
			
			// there can only be one change atom for a
			// variable in the send window. stop searching
			
			break;
		}
	}
	
	// check if there is an existing atom for this object in
	// the send queue
	
	atom = irmo_hash_table_lookup(client->sendq_hashtable,
				   (void *) object->id);

	if (!atom) {
		atom = irmo_new0(IrmoChangeAtom, 1);
		IRMO_SENDATOM(atom)->klass = &irmo_change_atom;

		atom->id = object->id;
		atom->object = object;
		atom->changed = irmo_new0(int, object->objclass->nvariables);
		atom->nchanged = 0;
		
		irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
	}

	// set the change in the atom and update the change count

	if (!atom->changed[variable]) {
		atom->changed[variable] = 1;
		++atom->nchanged;
	}
	
	// need to recalculate atom size

	atom->sendatom.len = irmo_change_atom.length(IRMO_SENDATOM(atom));
}

void irmo_client_sendq_add_destroy(IrmoClient *client, IrmoObject *object)
{
	IrmoDestroyAtom *atom;
	int i;
	
	// check for any changeatoms referring to this object

	atom = irmo_hash_table_lookup(client->sendq_hashtable,
				   (void *) object->id);

	// convert to a ATOM_NULL atom
	
	if (atom) {
		irmo_sendatom_nullify(IRMO_SENDATOM(atom));
		irmo_hash_table_remove(client->sendq_hashtable,
				    (void *) object->id);
	}

	// nullify atoms in send window too
	
	for (i=0; i<client->sendwindow_size; ++i) {
		IrmoChangeAtom *catom;

		if (client->sendwindow[i]->klass != &irmo_change_atom)
			continue;
		
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
	IrmoMethod *method = data->spec;
	IrmoMethodAtom *atom;
	int i;
	
	// create a new method atom
	
	atom = irmo_new0(IrmoMethodAtom, 1);
	atom->sendatom.klass = &irmo_method_atom;
	atom->method.spec = data->spec;

	// copy arguments

	atom->method.args = irmo_new0(IrmoValue, method->narguments);
	memcpy(atom->method.args,
	       data->args,
	       sizeof(IrmoValue) * method->narguments);

	// duplicate all the strings
	
	for (i=0; i<method->narguments; ++i) {
		if (method->arguments[i]->type == IRMO_TYPE_STRING)
			atom->method.args[i].s
				= strdup(atom->method.args[i].s);
	}

	// add to queue

	irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
}

// send a send window upper limit

void irmo_client_sendq_add_sendwindow(IrmoClient *client, int max)
{
	IrmoSendWindowAtom *atom;

	atom = irmo_new0(IrmoSendWindowAtom, 1);
	atom->sendatom.klass = &irmo_sendwindow_atom;

	atom->max = max;

	irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
}

// queue up the entire current world state in the client send queue
// this is used for when new clients connect to retrieve the entire
// current world state

static void client_sendq_add_objects(IrmoObject *object, IrmoClient *client)
{
	irmo_client_sendq_add_new(client, object);
}

static void client_sendq_add_variables(IrmoObject *object, 
				       IrmoClient *client)
{
	int i;

	// queue up variables

	for (i=0; i<object->objclass->nvariables; ++i)
		irmo_client_sendq_add_change(client, object, i);
}

void irmo_client_sendq_add_state(IrmoClient *client)
{
	// create all the objects first
	// this is done all together and seperately from sending the
	// variable state, as the rle encoding in the packets will
	// better compress the atoms this way
	irmo_world_foreach_object(client->server->world, NULL,
				     (IrmoObjCallback) client_sendq_add_objects,
				     client);

	// send variable states
	irmo_world_foreach_object(client->server->world, NULL,
				     (IrmoObjCallback) client_sendq_add_variables,
				     client);
}

