#ifndef IRMO_CONNECTION_H
#define IRMO_CONNECTION_H

typedef struct _IrmoConnection IrmoConnection;

#include "if_spec.h"
#include "server.h"
#include "universe.h"

IrmoConnection *irmo_connect(int domain, gchar *location, int port,
                             InterfaceSpec *spec,
			     IrmoUniverse *local_universe);


#endif /* #ifndef IRMO_CONNECTION_H */

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/06 02:10:11  sdh300
// Add missing CVS tags
//
