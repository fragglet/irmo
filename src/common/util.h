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

#ifndef IRMO_COMMON_UTIL_H
#define IRMO_COMMON_UTIL_H

#include <stdlib.h>
#include <string.h>

//
// Memory allocation macros
//

// allocate memory, zeroing out the contents first

void *irmo_malloc0(int bytes);

// allocate enough memory for an array of structures, giving the structure
// name and array size, and zeroing the contents of the array

#define irmo_new0(typename, count)                             \
        ((typename *) irmo_malloc0(sizeof(typename) * count))

// resize an array of structures

#define irmo_renew(typename, oldmem, count)                  \
        ((typename *) realloc((oldmem), sizeof(typename) * (count)))

//
// Debugging macros
//

// return if a test fails

#define irmo_return_if_fail(test)                                      \
        do {                                                           \
                if (!(test)) {                                         \
                        irmo_assert_fail_message(__FUNCTION__, #test); \
                        return;                                        \
                }                                                      \
        } while(0)

// return a value if a test fails

#define irmo_return_val_if_fail(test, val)                             \
        do {                                                           \
                if (!(test)) {                                         \
                        irmo_assert_fail_message(__FUNCTION__, #test); \
                        return (val);                                  \
                }                                                      \
        } while(0)

void irmo_assert_fail_message(const char *function_name, char *assertation);

#endif /* #ifndef IRMO_COMMON_UTIL_H */

