// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#include <stdlib.h>
#include <string.h>

#include "if_spec.h"
#include "object.h"
#include "sendatom.h"

static inline void sendatom_change_free_data(IrmoSendAtom *atom)
{
	int i;

	if (atom->data.change.newvalues) {
		ClassSpec *objclass = atom->data.change.objclass;

		for (i=0; i<objclass->nvariables; ++i) {
			// only changed values are stored
			
			if (!atom->data.change.changed[i])
				continue;
			
			// free strings
			
			if (objclass->variables[i]->type == TYPE_STRING)
				free(atom->data.change.newvalues[i].s);
		}

		free(atom->data.change.newvalues);
	}
	
	free(atom->data.change.changed);
}

static inline void sendatom_method_free_data(IrmoSendAtom *atom)
{
	MethodSpec *method = atom->data.method.spec;
	int i;

	for (i=0; i<method->narguments; ++i) {
		if (method->arguments[i]->type == TYPE_STRING)
			free(atom->data.method.args[i].s);
	}

	free(atom->data.method.args);
}

void sendatom_free(IrmoSendAtom *atom)
{
	if (atom->type == ATOM_CHANGE)
		sendatom_change_free_data(atom);
	if (atom->type == ATOM_METHOD)
		sendatom_method_free_data(atom);
	
	free(atom);
}

static void sendatom_nullify(IrmoSendAtom *atom)
{
	if (atom->type == ATOM_CHANGE)
		sendatom_change_free_data(atom);

	atom->type = ATOM_NULL;
}

static int sendatom_change_len(IrmoSendAtom *atom)
{
	IrmoObject *obj = atom->data.change.object;
	ClassSpec *spec = obj->objclass;
	int len;
	int i;

	len = 0;

	// object class

	len += 1;
	
	// object id

	len += 2;
	
	// leading bitmap
	
	len += (spec->nvariables + 7) / 8;

	// add up sizes of variables
	
	for (i=0; i<spec->nvariables; ++i) {

		// only variables which have changed
		
		if (!atom->data.change.changed[i])
			continue;
		
		switch (spec->variables[i]->type) {
		case TYPE_INT8:
			len += 1;
			break;
		case TYPE_INT16:
			len += 2;
			break;
		case TYPE_INT32:
			len += 4;
			break;
		case TYPE_STRING:
			len += strlen(obj->variables[i].s) + 1;
			break;
		}
	}

	return len;
}

void irmo_client_sendq_add_new(IrmoClient *client, IrmoObject *object)
{
	IrmoSendAtom *atom;

	atom = g_new0(IrmoSendAtom, 1);

	atom->type = ATOM_NEW;
	atom->data.newobj.id = object->id;
	atom->data.newobj.classnum = object->objclass->index;

	//   1 byte for class number
	// + 2 bytes for object id
	
	atom->len = 1 + 2;

	g_queue_push_tail(client->sendq, atom);
}

void irmo_client_sendq_add_change(IrmoClient *client,
				  IrmoObject *object, int variable)
{
	IrmoSendAtom *atom;
	int i, n;
	
	// search the send window and nullify this variable if there
	// is an existing change for it waiting to be acked

	for (i=0; i<client->sendwindow_size; ++i) {
		atom = client->sendwindow[i];

		// check this is a change atom for this variable in
		// this object
		
		if (atom->type == ATOM_CHANGE
		 && atom->data.change.object == object
		 && atom->data.change.changed[variable]) {

			// unset the change in the atom. update
			// change count
			
			atom->data.change.changed[variable] = FALSE;
			--atom->data.change.nchanged;

			// if there are no more changes, replace the atom
			// with a NULL

			if (atom->data.change.nchanged <= 0) {
				sendatom_nullify(atom);
			}
			
			// there can only be one change atom for a
			// variable in the send window. stop searching
			
			break;
		}
	}
	
	// check if there is an existing atom for this object in
	// the send queue
	
	atom = g_hash_table_lookup(client->sendq_hashtable,
				   (gpointer) object->id);

	if (!atom) {
		atom = g_new0(IrmoSendAtom, 1);
		atom->type = ATOM_CHANGE;
		atom->data.change.id = object->id;
		atom->data.change.object = object;
		atom->data.change.changed
			= g_new0(gboolean, object->objclass->nvariables);
		atom->data.change.nchanged = 0;
		
		g_queue_push_tail(client->sendq, atom);
		g_hash_table_insert(client->sendq_hashtable,
				    (gpointer) object->id,
				    atom);
	}

	// set the change in the atom and update the change count

	if (!atom->data.change.changed[variable]) {
		atom->data.change.changed[variable] = TRUE;
		++atom->data.change.nchanged;
	}
	
	// need to recalculate atom size

	atom->len = sendatom_change_len(atom);
}

void irmo_client_sendq_add_destroy(IrmoClient *client, IrmoObject *object)
{
	IrmoSendAtom *atom;
	int i;
	
	// check for any changeatoms referring to this object

	atom = g_hash_table_lookup(client->sendq_hashtable,
				   (gpointer) object->id);

	// convert to a ATOM_NULL atom
	
	if (atom) {
		sendatom_nullify(atom);
		g_hash_table_remove(client->sendq_hashtable,
				    (gpointer) object->id);
	}

	// nullify atoms in send window too
	
	for (i=0; i<client->sendwindow_size; ++i) {
		if (client->sendwindow[i]->type == ATOM_CHANGE
		    && object == client->sendwindow[i]->data.change.object) {
			sendatom_nullify(client->sendwindow[i]);
		}
	}
	
	// create a destroy atom

	atom = g_new0(IrmoSendAtom, 1);

	atom->type = ATOM_DESTROY;
	atom->data.destroy.id = object->id;

	// 2 bytes for object id to destroy
	
	atom->len = 2;

	g_queue_push_tail(client->sendq, atom);
}

void irmo_client_sendq_add_method(IrmoClient *client, IrmoMethodData *data)
{
	MethodSpec *method = data->spec;
	IrmoSendAtom *atom;
	int i;
	
	// create a new method atom
	
	atom = g_new0(IrmoSendAtom, 1);
	atom->type = ATOM_METHOD;
	atom->data.method.spec = data->spec;

	// copy arguments

	atom->data.method.args = g_new0(IrmoVariable, method->narguments);
	memcpy(atom->data.method.args,
	       data->args,
	       sizeof(IrmoVariable) * method->narguments);

	// find length of atom,

	atom->len = 0;

	// method number
	
	atom->len += 1;

	// find length of arguments
	// copy strings while we are here
	
	for (i=0; i<method->narguments; ++i) {
		switch (method->arguments[i]->type) {
		case TYPE_INT8:
			atom->len += 1;
			break;
		case TYPE_INT16:
			atom->len += 2;
			break;
		case TYPE_INT32:
			atom->len += 4;
			break;
		case TYPE_STRING:
			atom->data.method.args[i].s
				= strdup(atom->data.method.args[i].s);
			atom->len += strlen(atom->data.method.args[i].s) + 1;
			break;
		}
	}

	// add to queue

	g_queue_push_tail(client->sendq, atom);
}

// send a send window upper limit

void irmo_client_sendq_add_sendwindow(IrmoClient *client, int max)
{
	IrmoSendAtom *atom;

	atom = g_new0(IrmoSendAtom, 1);
	atom->type = ATOM_SENDWINDOW;
	atom->data.sendwindow.max = max;

	atom->len = 2;

	g_queue_push_tail(client->sendq, atom);
}

IrmoSendAtom *irmo_client_sendq_pop(IrmoClient *client)
{
	IrmoSendAtom *atom;

	while (1) {
	
		atom = (IrmoSendAtom *) g_queue_pop_head(client->sendq);
		
		if (!atom)
			return NULL;

		// automatically ignore and delete NULL atoms that
		// are in the sendq
		
		if (atom->type != ATOM_NULL)
			break;
		
		sendatom_free(atom);
	} 

	// if a change, remove from the change hash

	if (atom->type == ATOM_CHANGE) {
		g_hash_table_remove(client->sendq_hashtable,
				    (gpointer) atom->data.change.object->id);
	}

	return atom;
}

// queue up the entire current universe state in the client send queue
// this is used for when new clients connect to retrieve the entire
// current universe state

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

	irmo_universe_foreach_object(client->server->universe, NULL,
				     (IrmoObjCallback) client_sendq_add_objects,
				     client);

	// send variable states

	irmo_universe_foreach_object(client->server->universe, NULL,
				     (IrmoObjCallback) client_sendq_add_variables,
				     client);
}

// $Log$
// Revision 1.1  2003/06/09 21:33:25  fraggle
// Initial revision
//
// Revision 1.17  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.16  2003/05/04 00:28:14  sdh300
// Add ability to manually set the maximum sendwindow size
//
// Revision 1.15  2003/04/25 01:32:39  sdh300
// Remove useless debug code left in from previous commit
//
// Revision 1.14  2003/04/25 00:40:50  sdh300
// Nullifying of change atoms for out-of-date data in the send window
//
// Revision 1.13  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.12  2003/03/12 18:59:26  sdh300
// Remove/comment out some debug messages
//
// Revision 1.11  2003/03/07 12:17:17  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.10  2003/03/06 21:29:05  sdh300
// On connect, send the entire universe state to the client
//
// Revision 1.9  2003/03/06 20:43:11  sdh300
// Nullify sendatoms in the send window as well as the send queue
//
// Revision 1.8  2003/03/05 15:32:21  sdh300
// Add object class to change atoms to make their coding in packets
// unambiguous.
//
// Revision 1.7  2003/03/05 15:28:13  sdh300
// Add receive window and extra data for sendatoms in the receive window.
//
// Revision 1.6  2003/03/03 21:03:45  sdh300
// Fix bug in client_sendq_pop
//
// Revision 1.5  2003/02/27 02:26:39  sdh300
// Fix compile errors
//
// Revision 1.4  2003/02/27 02:07:56  sdh300
// Store sendatom size in structure
// Add 'pop' function to remove atoms from sendq head
//
// Revision 1.3  2003/02/20 18:24:59  sdh300
// Use GQueue instead of a GPtrArray for the send queue
// Initial change/destroy code
//
// Revision 1.2  2003/02/18 20:26:42  sdh300
// Initial send queue building/notification code
//
// Revision 1.1  2003/02/18 18:25:40  sdh300
// Initial queue object code
//
