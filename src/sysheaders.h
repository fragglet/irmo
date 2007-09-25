// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
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
//---------------------------------------------------------------------

#ifndef IRMO_SYSHEADERS_H
#define IRMO_SYSHEADERS_H

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

// Common algorithms and utility functions

#include "common/common.h"

// Architecture-specific code

#include "arch/arch.h"

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

extern char *g_strdup_printf(char *fmt, ...);
extern char *g_strdup_vprintf(char *fmt, va_list args);


#endif /* #ifndef IRMO_SYSHEADERS_H */

