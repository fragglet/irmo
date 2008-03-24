//
// Copyright (C) 2002-2008 Simon Howard
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

#ifndef IRMO_BASE_ASSERT_H
#define IRMO_BASE_ASSERT_H

#include <stdlib.h>
#include <string.h>

//
// Debugging macros.  
//
// Define IRMO_NO_DEBUG_CHECK to not check the parameters passed to API 
// functions for extra speed.
//

#ifdef IRMO_NO_DEBUG_CHECK

#define irmo_return_if_fail(test)
#define irmo_return_val_if_fail(test, val)

#else

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

#endif

#define irmo_bug()                                                     \
        irmo_bug_abort(__FILE__, __LINE__)

void irmo_warning_message(const char *function_name, char *message, ...);
void irmo_assert_fail_message(const char *function_name, char *assertation);
void irmo_bug_abort(char *file, int line);

#endif /* #ifndef IRMO_BASE_ASSERT_H */

