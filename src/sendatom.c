#include <stdlib.h>

#include "if_spec.h"
#include "object.h"
#include "sendatom.h"

void sendatom_free(IrmoSendAtom *atom)
{
	free(atom);
}

void client_sendq_add_new(IrmoClient *client, IrmoObject *object)
{
	IrmoSendAtom *atom;

	atom = g_new0(IrmoSendAtom, 1);

	atom->type = ATOM_NEW;
	atom->data.newobj.id = object->id;
	atom->data.newobj.classnum = object->objclass->index;

	g_ptr_array_add(client->sendq, atom);
}

void client_sendq_add_change(IrmoClient *client,
			     IrmoObject *object, int variable)
{
	printf("notify client of change to object %i:%i\n", object->id,
	       variable);
}

void client_sendq_add_destroy(IrmoClient *client, IrmoObject *object)
{
	printf("notify client of object %i destruction\n", object->id);
}

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/02/18 18:25:40  sdh300
// Initial queue object code
//
