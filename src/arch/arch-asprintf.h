// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2005 Simon Howard
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

#ifndef IRMO_ARCH_ASPRINTF_H
#define IRMO_ARCH_ASPRINTF_H

#include <stdarg.h>

// 
// Printf to string, allocating the string automatically to the size 
// needed.
//

char *irmo_asprintf(char *format, ...);

//
// Printf to string, as irmo_asprintf, taking a varargs list.
//

char *irmo_vasprintf(char *format, va_list args);


#endif /* #ifndef IRMO_ARCH_ASPRINTF_H */

