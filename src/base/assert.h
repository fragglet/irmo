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

/*!
 * Assertation check.  Return from the current function if the given
 * test fails.  A warning message is printed using
 * @param irmo_assert_fail_message.
 *
 * @param test       The assertation.
 */

#define irmo_return_if_fail(test)                                      \
        do {                                                           \
                if (!(test)) {                                         \
                        irmo_assert_fail_message(__FUNCTION__, #test); \
                        return;                                        \
                }                                                      \
        } while(0)

/*!
 * Assertation check.  Return from the current function if the given
 * test fails, returning a value.  A warning message is printed using
 * @param irmo_assert_fail_message.
 *
 * @param test       The assertation.
 * @param val        The value to return.
 */

#define irmo_return_val_if_fail(test, val)                             \
        do {                                                           \
                if (!(test)) {                                         \
                        irmo_assert_fail_message(__FUNCTION__, #test); \
                        return (val);                                  \
                }                                                      \
        } while(0)

#endif

/*!
 * Abort the program due to an internal bug.
 */

#define irmo_bug()                                                     \
        irmo_bug_abort(__FILE__, __LINE__)

/*!
 * Display a warning message due to an improper use of a function.
 *
 * @param function_name     Name of the function that was called.
 * @param message           String containing the message.
 */

void irmo_warning_message(const char *function_name, char *message, ...);

/*!
 * Display a warning message due to a failed assertation.
 *
 * @param function_name     Name of the function where the failed assertation
 *                          occurred.
 * @param assertation       The assertation that failed.
 */

void irmo_assert_fail_message(const char *function_name, char *assertation);

/*!
 * Quit the program due to an internal error in the library.
 *
 * @param file              The file where the abort occurred.
 * @param line              The line of the file where the abort occurred.
 */

void irmo_bug_abort(char *file, int line);

#endif /* #ifndef IRMO_BASE_ASSERT_H */

