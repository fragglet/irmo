#ifndef IRMO_SENDATOM_H
#define IRMO_SENDATOM_H

typedef struct _IrmoSendAtom IrmoSendAtom;

#include "client.h"

// queue object

struct _IrmoSendAtom {
	enum {
		ATOM_NEW,                // new object 
		ATOM_CHANGE,             // modified object 
		ATOM_DESTROY,            // object destroyed 
		ATOM_METHOD,             // method call
	} type;

	union {
		struct {
			irmo_objid_t id;
			guint classnum;
		} newobj;
		struct {
			irmo_objid_t id;

			// more complex stuff later:
			// variables changed, new values etc.
		} change;
		struct {
			irmo_objid_t id;
		} destroy;
		struct {
			// will come later
		} method;
	} data;
};

void sendatom_free(IrmoSendAtom *atom);

// create a new sendatom and add to a clients sendqueue

void client_sendq_add_new(IrmoClient *client, IrmoObject *object);
void client_sendq_add_change(IrmoClient *client,
			     IrmoObject *object, int variable);
void client_sendq_add_destroy(IrmoClient *client, IrmoObject *object);
       

#endif /* #ifndef IRMO_SENDATOM_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/02/18 18:25:40  sdh300
// Initial queue object code
//

