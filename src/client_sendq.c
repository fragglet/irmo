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

#include "netlib.h"

#include "if_spec.h"
#include "object.h"
#include "sendatom.h"

IrmoSendAtom *irmo_client_sendq_pop(IrmoClient *client)
{
	IrmoSendAtom *atom;

	while (1) {
	
		atom = (IrmoSendAtom *) g_queue_pop_head(client->sendq);
		
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

		g_hash_table_remove(client->sendq_hashtable,
				    (gpointer) catom->object->id);
	}

	return atom;
}

void irmo_client_sendq_push(IrmoClient *client, IrmoSendAtom *atom)
{
	if (atom->klass == &irmo_change_atom) {
		IrmoChangeAtom *catom = (IrmoChangeAtom *) atom;

		g_hash_table_insert(client->sendq_hashtable,
				    (gpointer) catom->object->id,
				    atom);
	}
	
	atom->client = client;
	atom->len = atom->klass->length(atom);
	
	g_queue_push_tail(client->sendq, atom);
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

	atom = g_new0(IrmoNewObjectAtom, 1);

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
			
			atom->changed[variable] = FALSE;
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
	
	atom = g_hash_table_lookup(client->sendq_hashtable,
				   (gpointer) object->id);

	if (!atom) {
		atom = g_new0(IrmoChangeAtom, 1);
		IRMO_SENDATOM(atom)->klass = &irmo_change_atom;

		atom->id = object->id;
		atom->object = object;
		atom->changed = g_new0(gboolean, object->objclass->nvariables);
		atom->nchanged = 0;
		
		irmo_client_sendq_push(client, IRMO_SENDATOM(atom));
	}

	// set the change in the atom and update the change count

	if (!atom->changed[variable]) {
		atom->changed[variable] = TRUE;
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

	atom = g_hash_table_lookup(client->sendq_hashtable,
				   (gpointer) object->id);

	// convert to a ATOM_NULL atom
	
	if (atom) {
		irmo_sendatom_nullify(IRMO_SENDATOM(atom));
		g_hash_table_remove(client->sendq_hashtable,
				    (gpointer) object->id);
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

	atom = g_new0(IrmoDestroyAtom, 1);
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
	
	atom = g_new0(IrmoMethodAtom, 1);
	atom->sendatom.klass = &irmo_method_atom;
	atom->method.spec = data->spec;

	// copy arguments

	atom->method.args = g_new0(IrmoValue, method->narguments);
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

	atom = g_new0(IrmoSendWindowAtom, 1);
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

// $Log$
// Revision 1.3  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.2  2003/11/20 00:19:17  fraggle
// Add some fixes to get compiling under windows
//
// Revision 1.1  2003/10/14 22:12:49  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
// Revision 1.8  2003/10/14 00:53:43  fraggle
// Remove pointless inlinings
//
// Revision 1.7  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.6  2003/09/01 14:21:20  fraggle
// Use "world" instead of "universe". Rename everything.
//
// Revision 1.5  2003/08/31 22:51:22  fraggle
// Rename IrmoVariable to IrmoValue and make public. Replace i8,16,32 fields
// with a single integer field. Add irmo_universe_method_call2 to invoke
// a method taking an array of arguments instead of using varargs
//
// Revision 1.4  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.3  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.2  2003/08/18 01:23:14  fraggle
// Use G_INLINE_FUNC instead of inline for portable inline function support
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
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
