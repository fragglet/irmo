//
// run sendatoms in receive window
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "client.h"
#include "object.h"
#include "sendatom.h"
#include "universe.h"

static void client_run_new(IrmoClient *client, IrmoSendAtom *atom)
{
	IrmoInterfaceSpec *spec = client->universe->spec;
	ClassSpec *objclass = spec->classes[atom->data.newobj.classnum];
	
	// sanity check

	if (irmo_universe_get_object_for_id(client->universe,
					    atom->data.newobj.id)) {
		fprintf(stderr,
			"client_run_new: new object id of %i but an object "
			"with that id already exists!\n",
			atom->data.newobj.id);
		return;
	}
	
	// create new object
							  
	irmo_object_internal_new(client->universe, objclass,
				 atom->data.newobj.id);
}

static void client_run_change(IrmoClient *client, IrmoSendAtom *atom)
{
	IrmoObject *obj;
	ClassSpec *objclass;
	IrmoVariable *newvalues;
	int i;

	// sanity checks

	obj = irmo_universe_get_object_for_id(client->universe,
					      atom->data.change.id);

	if (!obj) {
		fprintf(stderr,
			"client_run_change: change to object %i but object "
			"does not exist!\n",
			atom->data.change.id);
		return;
	}

	if (obj->objclass != atom->data.change.objclass) {
		fprintf(stderr,
			"client_run_change: conflicting object classes for "
			"change (%s, %s)\n",
			obj->objclass->name,
			atom->data.change.objclass->name);
		return;	       
	}

	objclass = obj->objclass;
	newvalues = atom->data.change.newvalues;
	
	// run through variables and apply changes
	
	for (i=0; i<obj->objclass->nvariables; ++i) {

		// not changed?
		
		if (!atom->data.change.changed[i])
			continue;

		// apply change

		switch (objclass->variables[i]->type) {
		case TYPE_INT8:
			obj->variables[i].i8 = newvalues[i].i8;
			break;
		case TYPE_INT16:
			obj->variables[i].i16 = newvalues[i].i16;
			break;
		case TYPE_INT32:
			obj->variables[i].i32 = newvalues[i].i32;
			break;
		case TYPE_STRING:
			free(obj->variables[i].s);
			obj->variables[i].s = strdup(newvalues[i].s);
			break;
		}

		irmo_object_set_raise(obj, i);
	}
}

static void client_run_destroy(IrmoClient *client, IrmoSendAtom *atom)
{
	IrmoObject *obj;

	// sanity check

	obj = irmo_universe_get_object_for_id(client->universe,
					      atom->data.destroy.id);

	if (!obj) {
		fprintf(stderr,
			"client_run_destroy: destroy object %i, but object "
			"does not exist!\n",
			atom->data.destroy.id);
		return;
	}

	// destroy object. remove from universe and call notify functions
	
	irmo_object_internal_destroy(obj, TRUE, TRUE);
}

void irmo_client_run_recvwindow(IrmoClient *client)
{
	int i;

	// nothing to run?
	
	if (!client->recvwindow[0])
		return;
	
	// run as many from the start as possible
	
	for (i=0;
	     i<client->recvwindow_size && client->recvwindow[i];
	     ++i) {
		IrmoSendAtom *atom = client->recvwindow[i];

		printf("run atom %i\n", i+client->recvwindow_start);

		switch (atom->type) {
		case ATOM_NULL:
			break;
		case ATOM_NEW:
			client_run_new(client, atom);
			break;
		case ATOM_CHANGE:
			client_run_change(client, atom);
			break;
		case ATOM_DESTROY:
			client_run_destroy(client, atom);
			break;
		case ATOM_METHOD:
			// todo			
			break;
		}

		sendatom_free(atom);
	}
	
	// move recvwindow along

	client->recvwindow_start += i;
	
	memcpy(client->recvwindow,
	       client->recvwindow + i,
	       sizeof(*client->recvwindow) * (client->recvwindow_size-i));

	// clear the end
	
	for (; i<client->recvwindow_size; ++i)
		client->recvwindow[i] = NULL;
}

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/03/06 19:33:50  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.1  2003/03/06 19:23:13  sdh300
// Add initial code to run through the atoms in the send window
//
