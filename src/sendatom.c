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

void sendatom_free(IrmoSendAtom *atom)
{
	if (atom->type == ATOM_CHANGE)
		sendatom_change_free_data(atom);
	
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
	
	atom = g_hash_table_lookup(client->sendq_hashtable,
				   (gpointer) object->id);

	if (!atom) {
		atom = g_new0(IrmoSendAtom, 1);
		atom->type = ATOM_CHANGE;
		atom->data.change.id = object->id;
		atom->data.change.object = object;
		atom->data.change.changed
			= g_new0(gboolean, object->objclass->nvariables);

		g_queue_push_tail(client->sendq, atom);
		g_hash_table_insert(client->sendq_hashtable,
				    (gpointer) object->id,
				    atom);
	}

	atom->data.change.changed[variable] = TRUE;

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
		printf("nullify atom\n");
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

// $Log: not supported by cvs2svn $
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
