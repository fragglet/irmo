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

#endif /* #ifndef IRMO_SENDATOM_H */

// $Log: not supported by cvs2svn $

