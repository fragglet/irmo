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

//
// Iterators
//

#ifndef IRMO_ITERATOR_H
#define IRMO_ITERATOR_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 *
 * In various places in the library it is possible to iterate over a number
 * of objects.  This interface provides a generic iterator interface for
 * this purpose through the \ref IrmoIterator object.
 *
 * \addtogroup iterator
 * \{
 */

/*!
 * Returns true if there are more objects to iterate over.
 *
 * \param iterator       The iterator.
 * \return               Non-zero if there are more objects to iterate over,
 *                       zero if there are no more objects to iterate over.
 */

int irmo_iterator_has_more(IrmoIterator *iter);

/*!
 * Get the next object from the collection.
 *
 * \param iterator       The iterator.
 * \return               A pointer to the object, or NULL if there are no 
 *                       more objects to iterate over.
 */

void *irmo_iterator_next(IrmoIterator *iter);

/*!
 * Free an iterator.  This must be called once iteration has completed.
 *
 * \param iterator       The iterator.
 */

void irmo_iterator_free(IrmoIterator *iter);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ITERATOR_H */

