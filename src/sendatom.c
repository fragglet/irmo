#include <stdlib.h>

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

void client_sendq_add_new(IrmoClient *client, IrmoObject *object)
{
	IrmoSendAtom *atom;

	atom = g_new0(IrmoSendAtom, 1);

	atom->type = ATOM_NEW;
	atom->data.newobj.id = object->id;
	atom->data.newobj.classnum = object->objclass->index;

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
}

void client_sendq_add_destroy(IrmoClient *client, IrmoObject *object)
{
	IrmoSendAtom *atom;
	
	// check for any changeatoms referring to this object

	atom = g_hash_table_lookup(client->sendq_hashtable,
				   (gpointer) object->id);

	// convert to a ATOM_NULL atom
	
	if (atom) {
		sendatom_nullify(atom);
		g_hash_table_remove(client->sendq_hashtable,
				    (gpointer) object->id);
	}
	
	// create a destroy atom

	atom = g_new0(IrmoSendAtom, 1);

	atom->type = ATOM_DESTROY;
	atom->data.destroy.id = object->id;

	g_queue_push_tail(client->sendq, atom);
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/18 20:26:42  sdh300
// Initial send queue building/notification code
//
// Revision 1.1  2003/02/18 18:25:40  sdh300
// Initial queue object code
//
