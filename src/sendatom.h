#ifndef IRMO_SENDATOM_H
#define IRMO_SENDATOM_H

typedef struct _IrmoSendAtom IrmoSendAtom;

#include <sys/time.h>

#include "client.h"

// queue object

struct _IrmoSendAtom {
	struct timeval sendtime;        // time this atom was last sent
	
	int len;			// length in packet
	enum {
		ATOM_NULL,               // null atom for nullified changes
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
			IrmoObject *object;
			gboolean *changed;  // array saying which have changed
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
       
IrmoSendAtom *client_sendq_pop(IrmoClient *client);

#endif /* #ifndef IRMO_SENDATOM_H */

// $Log: not supported by cvs2svn $
// Revision 1.4  2003/02/27 02:07:56  sdh300
// Store sendatom size in structure
// Add 'pop' function to remove atoms from sendq head
//
// Revision 1.3  2003/02/20 18:25:00  sdh300
// Use GQueue instead of a GPtrArray for the send queue
// Initial change/destroy code
//
// Revision 1.2  2003/02/18 20:26:42  sdh300
// Initial send queue building/notification code
//
// Revision 1.1  2003/02/18 18:25:40  sdh300
// Initial queue object code
//

