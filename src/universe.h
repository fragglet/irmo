//
// Irmo Universe
//

#ifndef IRMO_INTERNAL_UNIVERSE_H
#define IRMO_INTERNAL_UNIVERSE_H

#include "public/universe.h"

#include "callback.h"
#include "client.h"
#include "if_spec.h"
#include "object.h"

// internals:

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoUniverse {

	// specification this universe implements
	
	IrmoInterfaceSpec *spec;

	// universe-global callback objects, 1 per class
	
	IrmoCallbackData **callbacks;

	// objects in the universe, hashed by their object id
	
	GHashTable *objects;

	// the id of the last object created. objects are created
	// with sequential ids
	
	irmo_objid_t lastid;

	// servers attached to this universe who are serving it.
	
	GPtrArray *servers;

	// number of references to this universe
	
	int refcount;

	// if true, this is a local copy of a remote universe and
	// cannot be changed
	
	gboolean remote;

	// if this is remote, this is the client from which we are
	// getting the universe state

	IrmoClient *remote_client;

	// method callbacks

	GSList **method_callbacks;
};

#endif /* #ifndef IRMO_INTERNAL_UNIVERSE_H */

// $Log: not supported by cvs2svn $
// Revision 1.14  2003/03/14 16:53:46  sdh300
// Add structure member for source client for remote universes
//
// Revision 1.13  2003/03/06 20:46:46  sdh300
// Add 'remote' flag. Add documentation for IrmoUniverse data.
//
// Revision 1.12  2003/03/06 19:33:51  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.11  2003/02/23 00:00:04  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
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
