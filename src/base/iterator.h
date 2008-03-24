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

#ifndef IRMO_BASE_ITERATOR_H
#define IRMO_BASE_ITERATOR_H

#include "algo/algo.h"

#include <irmo/iterator.h>

// Internal interface for iterators.

/*!
 * Function used to filter values returned from an iterator.
 *
 * @param value            The value to be returned by the iterator.
 * @param user_data        Extra user-specified data for implementing 
 *                         the filter.
 * @return                 Non-zero if the value should be returned from
 *                         the iterator, zero if the value should be 
 *                         ignored (filtered).
 */

typedef int (*IrmoIteratorFilter)(void *value, void *user_data);

/*!
 * Iterate over values in a @ref IrmoHashTable.
 *
 * @param hashtable        The hash table to iterate over.
 * @return                 A new @ref IrmoIterator object to iterate over
 *                         values in the hash table.
 */

IrmoIterator *irmo_iterate_hash_table(IrmoHashTable *hashtable);

/*!
 * Iterate over an array of objects.
 *
 * @param array            The array.
 * @param length           Length of the array.
 * @return                 A new @ref IrmoIterator object to iterate over
 *                         values in the array.
 */

IrmoIterator *irmo_iterate_array(void **array, int length);

/*!
 * Set a filter function to filter values being iterated over.
 *
 * @param iter             The iterator.
 * @param callback         Callback function to determine whether to pass
 *                         or filter values.
 * @param callback_data    User-specified data to be passed to the callback
 *                         function.
 */

void irmo_iterator_set_filter(IrmoIterator *iter,
                              IrmoIteratorFilter callback,
                              void *callback_data);

#endif /* #ifndef IRMO_BASE_ITERATOR_H */

