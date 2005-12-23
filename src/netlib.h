// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#ifndef IRMO_NETLIB_H
#define IRMO_NETLIB_H

#include "socket.h"

// various network util functions

int irmo_sockaddr_len(int domain);

unsigned int irmo_sockaddr_hash(struct sockaddr *addr);
int irmo_sockaddr_cmp(struct sockaddr *a, struct sockaddr *b);

struct sockaddr *irmo_sockaddr_copy(struct sockaddr *addr);

struct sockaddr *irmo_sockaddr_for_name(IrmoSocketDomain domain, 
					char *name, int port);

// time functions

// add two timevals (result = a + b)

void irmo_timeval_add(GTimeVal *a, GTimeVal *b,
		      GTimeVal *result);

// subtract timevals (result = a - b)

void irmo_timeval_sub(GTimeVal *a, GTimeVal *b,
		      GTimeVal *result);

// compare timevals

int irmo_timeval_cmp(GTimeVal *a, GTimeVal *b);

// conversion to milliseconds

int irmo_timeval_to_ms(GTimeVal *a);
void irmo_timeval_from_ms(int ms, GTimeVal *a);

#endif /* #ifndef IRMO_NETLIB_H */

// $Log$
// Revision 1.10  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.9  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.8  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.7  2003/11/18 19:32:19  fraggle
// Use GTimeVal instead of struct timeval
//
// Revision 1.6  2003/11/18 19:24:22  fraggle
// Fix compile errors and warnings introduced by previous commit
//
// Revision 1.5  2003/11/18 18:14:46  fraggle
// Get compilation under windows to work, almost
//
// Revision 1.4  2003/11/17 00:27:34  fraggle
// Remove glib dependency in API
//
// Revision 1.3  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.2  2003/08/26 14:57:31  fraggle
// Remove AF_* BSD sockets dependency from Irmo API
//
// Revision 1.1.1.1  2003/06/09 21:33:24  fraggle
// Initial sourceforge import
//
// Revision 1.8  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.7  2003/03/18 20:55:46  sdh300
// Initial round trip time measurement
//
// Revision 1.6  2003/02/06 01:58:16  sdh300
// Abstracted address resolve function
//
// Revision 1.5  2003/02/03 20:54:01  sdh300
// sockaddr copy function
//
// Revision 1.4  2003/02/03 20:44:20  sdh300
// move sockaddr_len into netlib
//
// Revision 1.3  2002/11/26 16:30:44  sdh300
// need ipv6 check around ipv6 sockaddr functions
//
// Revision 1.2  2002/11/26 16:26:15  sdh300
// missing glib.h include
//
// Revision 1.1  2002/11/26 16:23:28  sdh300
// Split off sockaddr hash functions to a seperate netlib module
//
