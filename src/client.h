#ifndef IRMO_CLIENT_H
#define IRMO_CLIENT_H

typedef struct _IrmoClient IrmoClient;

#include <netinet/in.h>

#include "server.h"
#include "socket.h"
#include "universe.h"

typedef enum {
	CLIENT_CONNECTING,             /* received first syn, sent syn ack */
	CLIENT_CONNECTED,              /* received syn ack reply, normal op */
	CLIENT_DISCONNECTED,           /* killed with syn fin */
} IrmoClientState;

struct _IrmoClient {

	IrmoClientState state;
	
	// server client is connected to

	IrmoServer *server;
	
	// clients remote universe
	
	IrmoUniverse *universe;

	// address:

	struct sockaddr *addr;
};

IrmoClient *_client_new(IrmoServer *server, struct sockaddr *addr);

/*!
 * \brief Forcibly disconnect a client
 *
 * \param client      The client to disconnect
 *
 */

void client_disconnect(IrmoClient *client);

#endif /* #ifndef IRMO_CLIENT_H */

// $Log: not supported by cvs2svn $
// Revision 1.2  2003/02/03 20:57:22  sdh300
// Initial client code
//
// Revision 1.1  2002/11/26 15:23:49  sdh300
// Initial network code
//

