//
// Irmo Universe
//

#ifndef IRMO_INTERNAL_UNIVERSE_H
#define IRMO_INTERNAL_UNIVERSE_H

#include "public/universe.h"

#include "callback.h"
#include "if_spec.h"
#include "object.h"

// internals:

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoUniverse {
	InterfaceSpec *spec;
	IrmoCallbackData **callbacks;
	GHashTable *objects;
	irmo_objid_t lastid;
	GPtrArray *servers;
	int refcount;
};

#endif /* #ifndef IRMO_INTERNAL_UNIVERSE_H */

// $Log: not supported by cvs2svn $
// Revision 1.10  2003/02/18 18:39:54  sdh300
// store list of attached servers in universe
//
// Revision 1.9  2002/11/17 20:37:15  sdh300
// initial doxygen documentation
//
// Revision 1.8  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.7  2002/11/13 13:56:23  sdh300
// add some documentation
//
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/21 14:58:07  sdh300
// split off object code to a seperate module
//
// Revision 1.4  2002/10/21 14:48:54  sdh300
// oops, fix build
//
// Revision 1.3  2002/10/21 14:43:27  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
