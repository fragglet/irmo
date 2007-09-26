//
// Copyright (C) 2002-3 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

#ifndef IRMO_NETLIB_H
#define IRMO_NETLIB_H

#include "net/socket.h"

// various network util functions

int irmo_sockaddr_len(int domain);

unsigned int irmo_sockaddr_hash(struct sockaddr *addr);
int irmo_sockaddr_cmp(struct sockaddr *a, struct sockaddr *b);

struct sockaddr *irmo_sockaddr_copy(struct sockaddr *addr);

struct sockaddr *irmo_sockaddr_for_name(IrmoSocketDomain domain, 
					char *name, int port);

#endif /* #ifndef IRMO_NETLIB_H */

