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

#ifndef IRMO_SYSHEADERS_H
#define IRMO_SYSHEADERS_H

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "common/common.h"

// C99 integer types

#ifdef _MSC_VER

typedef unsigned char uint8_t;
typedef signed char int8_t;
typedef unsigned short uint16_t;
typedef signed short int16_t;
typedef unsigned long uint32_t;
typedef signed long int32_t;

#else 

#include <stdint.h>

#endif

// sockets api

#ifdef _WIN32

#include <WinSock.h>

#else

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <fcntl.h>

#define closesocket close

#endif

#include <glib.h>

#endif /* #ifndef IRMO_SYSHEADERS_H */

// $Log$
// Revision 1.6  2005/12/25 00:49:50  fraggle
// Undo last commit; add back glib.h
//
// Revision 1.5  2005/12/25 00:48:29  fraggle
// Use internal memory functions, rather than the glib ones
//
// Revision 1.4  2005/12/24 00:47:21  fraggle
// Include the autotools config.h header so that config settings get included.
//
// Revision 1.3  2005/12/24 00:15:59  fraggle
// Use the C99 "uintN_t" standard integer types rather than the glib
// guint types.
//
// Revision 1.2  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.1  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.4  2003/10/18 01:34:45  fraggle
// Better error reporting for connecting, allow server to send back an
// error message when refusing connections
//
// Revision 1.3  2003/10/17 23:33:05  fraggle
// protocol version checking
//
// Revision 1.2  2003/09/03 15:28:30  fraggle
// Add irmo_ prefix to all internal global functions (namespacing)
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.4  2003/06/09 21:06:52  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.3  2003/03/21 17:21:46  sdh300
// Round Trip Time estimatation and adaptive timeout times
//
// Revision 1.2  2003/03/14 01:07:23  sdh300
// Initial packet verification code
//
// Revision 1.1  2003/03/07 12:31:51  sdh300
// Add protocol.h
//

