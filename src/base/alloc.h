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

#ifndef IRMO_BASE_ALLOC_H
#define IRMO_BASE_ALLOC_H

#include <stdlib.h>

//
// Memory allocation macros
//

/*!
 * Generate an error message and abort due to a memory allocation
 * failure.
 *
 * @param bytes          Number of bytes that failed to be allocated,
 *                       or zero to generate a generic error message.
 */

void irmo_alloc_fail(size_t bytes);

/*!
 * Allocate memory, zeroing out the contents first.
 *
 * @param bytes          Number of bytes to allocate.
 * @return               Pointer to a new memory buffer.
 */

void *irmo_malloc0(size_t bytes);

/*!
 * Reallocate a buffer to a different size.
 *
 * @param ptr            Pointer to the existing buffer.
 * @param bytes          Size of the new buffer, in bytes.
 * @return               Pointer to the new buffer.
 */

void *irmo_realloc(void *ptr, size_t bytes);

/*!
 * Check the provided condition, and abort with an out of memory error
 * if it does not evaluate to true.
 *
 * @param condition  The condition to check.
 */

#define irmo_alloc_assert(condition)                           \
        do {                                                   \
                if (!(condition)) {                            \
                        irmo_alloc_fail(0);                    \
                }                                              \
        } while (0)

/*!
 * Allocate enough memory for an array of structures, giving the structure
 * name and array size, and zeroing the contents of the array.
 *
 * @param typename   The type of structure.
 * @param count      Number of structures to allocate.
 * @return           Pointer to a new memory buffer for the array.
 */

#define irmo_new0(typename, count)                             \
        ((typename *) irmo_malloc0(sizeof(typename) * count))

/*!
 * Resize an array of structures.
 *
 * @param typename   The type of structure.
 * @param oldmem     The old memory buffer.
 * @param count      New number of structures.
 * @return           Pointer to a new memory buffer for the array, containing
 *                   the contents of the previous buffer.
 */

#define irmo_renew(typename, oldmem, count)                  \
        ((typename *) irmo_realloc((oldmem), sizeof(typename) * (count)))

#endif /* #ifndef IRMO_BASE_ALLOC_H */

