#include <stdlib.h>
#include <string.h>

#include "if_spec.h"
#include "object.h"
#include "sendatom.h"

void sendatom_free(IrmoSendAtom *atom)
{
	if (atom->type == ATOM_CHANGE) {
		free(atom->data.change.changed);
	}
	
	free(atom);
}

static void sendatom_nullify(IrmoSendAtom *atom)
{
	if (atom->type == ATOM_CHANGE) {
		free(atom->data.change.changed);
	}

	atom->type = ATOM_NULL;
}

static int sendatom_change_len(IrmoSendAtom *atom)
{
	IrmoObject *obj = atom->data.change.object;
	ClassSpec *spec = obj->objclass;
	int len;
	int i;

	len = 0;

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

void client_sendq_add_new(IrmoClient *client, IrmoObject *object)
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

void client_sendq_add_change(IrmoClient *client,
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

void client_sendq_add_destroy(IrmoClient *client, IrmoObject *object)
{
	IrmoSendAtom *atom;
	
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
	
	// create a destroy atom

	atom = g_new0(IrmoSendAtom, 1);

	atom->type = ATOM_DESTROY;
	atom->data.destroy.id = object->id;

	// 2 bytes for object id to destroy
	
	atom->len = 2;
	
	g_queue_push_tail(client->sendq, atom);
}

IrmoSendAtom *client_sendq_pop(IrmoClient *client)
{
	IrmoSendAtom *atom;

	do {
	
		atom = (IrmoSendAtom *) g_queue_pop_head(client->sendq);
		
		if (!atom)
			return NULL;

		// automatically ignore and delete NULL atoms that
		// are in the sendq
		
		if (atom->type == ATOM_NULL) {
			sendatom_free(atom);
			continue;
		}
	} while (0);

	// if a change, remove from the change hash

	if (atom->type == ATOM_CHANGE) {
		g_hash_table_remove(client->sendq_hashtable,
				    (gpointer) atom->data.change.object->id);
	}

	return atom;
}

// $Log: not supported by cvs2svn $
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
