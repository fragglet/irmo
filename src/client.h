#ifndef IRMO_CLIENT_H
#define IRMO_CLIENT_H

typedef struct _IrmoClient IrmoClient;

#include <netinet/in.h>

#include "server.h"
#include "socket.h"
#include "universe.h"

struct _IrmoClient {

	// server client is connected to

	IrmoServer *server;
	
	// clients remote universe
	
	IrmoUniverse *universe;

	// address:

	struct sockaddr *addr;
};


/*!
 * \brief Forcibly disconnect a client
 *
 * \param client      The client to disconnect
 *
 */

void client_disconnect(IrmoClient *client);

#endif /* #ifndef IRMO_CLIENT_H */

// $Log: not supported by cvs2svn $

