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

#include "arch/sysheaders.h"

#include "base/util.h"

#include "iterator.h"

typedef void *(*IrmoIteratorNextCallback)(void *data);
typedef int (*IrmoIteratorHasMoreCallback)(void *data);
typedef void (*IrmoIteratorFreeCallback)(void *data);

typedef struct _IrmoIteratorType IrmoIteratorType;

struct _IrmoIteratorType {
        IrmoIteratorNextCallback get_next;
        IrmoIteratorHasMoreCallback has_more;
        IrmoIteratorFreeCallback free;
};

typedef enum {
        // Initial status.
        ITERATOR_INIT,

        // In the process of iterating.
        ITERATOR_RUNNING,

        // Iteration has completed.
        ITERATOR_FINISHED,
} IrmoIteratorStatus;

struct _IrmoIterator {

        // Current status

        IrmoIteratorStatus status;

        // Type of the iterator.  Each type of iterator has a
        // \ref IrmoIteratorType structure defined for it, which 
        // specifies callback functions to be invoked for each 
        // of the methods that may be invoked on an iterator.

        const IrmoIteratorType *type;

        // This is extra data that should be passed to the 
        // callback functions when invoked.

        void *data;

        // Next value to be returned by the iterator.

        void *next_value;

        // Filter function to invoke to filter values returned, or
        // NULL if no filter is set.

        IrmoIteratorFilter filter;
        void *filter_data;
};

//
// Hash table iterator
//

static const IrmoIteratorType hash_table_iterator = {
        (IrmoIteratorNextCallback) irmo_hash_table_iter_next,
        (IrmoIteratorHasMoreCallback) irmo_hash_table_iter_has_more,
        (IrmoIteratorFreeCallback) irmo_hash_table_iter_free,
};

IrmoIterator *irmo_iterate_hash_table(IrmoHashTable *hashtable)
{
        IrmoIterator *result;
        IrmoHashTableIterator *iter;

        iter = irmo_hash_table_iterate(hashtable);

        result = irmo_new0(IrmoIterator, 1);

        result->type = &hash_table_iterator;
        result->data = iter;

        return result;
}

//
// Array iterator
//

typedef struct _IrmoArrayIterator IrmoArrayIterator;

struct _IrmoArrayIterator {
        IrmoIterator iterator;
        void **array;
        int array_len;
        int position;
};

static void *irmo_array_iterator_next(void *data)
{
        IrmoArrayIterator *iter = data;
        void *result;

        if (iter->position >= iter->array_len) {
                result = NULL;
        } else {
                result = iter->array[iter->position];
                ++iter->position;
        }

        return result;
}

static int irmo_array_iterator_has_more(void *data)
{
        IrmoArrayIterator *iter = data;

        return iter->position < iter->array_len;
}

static const IrmoIteratorType array_iterator = {
        irmo_array_iterator_next,
        irmo_array_iterator_has_more,
        NULL,
};

IrmoIterator *irmo_iterate_array(void **array, int length)
{
        IrmoArrayIterator *result;

        result = irmo_new0(IrmoArrayIterator, 1);

        result->iterator.type = &array_iterator;
        result->iterator.data = result;
        result->array = array;
        result->array_len = length;
        result->position = 0;

        return &result->iterator;
}

//
// Generic functions
//

// Advance to the next value to iterate, and save it in iter->next_value.

static void irmo_iterator_advance(IrmoIterator *iter)
{
        iter->status = ITERATOR_RUNNING;

        do {
                // Have we reached the end?

                if (!iter->type->has_more(iter->data)) {
                        iter->status = ITERATOR_FINISHED;
                        break;
                }

                // Get the next value

                iter->next_value = iter->type->get_next(iter->data);

                // This value may be filtered.  Keep on going until
                // we get a non-filtered value.

        } while (iter->filter != NULL 
              && !iter->filter(iter->next_value, iter->filter_data));
}

int irmo_iterator_has_more(IrmoIterator *iter)
{
        if (iter->status == ITERATOR_INIT) {
                irmo_iterator_advance(iter);
        }

        return iter->status == ITERATOR_RUNNING;
}

void *irmo_iterator_next(IrmoIterator *iter)
{
        void *result;

        if (iter->status == ITERATOR_INIT) {
                irmo_iterator_advance(iter);
        }

        if (iter->status == ITERATOR_FINISHED) {
                // Have iterated over the entire set.

                result = NULL;
        } else {
                // Save the next value and advance.

                result = iter->next_value;

                irmo_iterator_advance(iter);
        }

        return result;
}

void irmo_iterator_free(IrmoIterator *iter)
{
        if (iter->type->free != NULL) {
                iter->type->free(iter->data);
        }

        free(iter);
}

void irmo_iterator_set_filter(IrmoIterator *iter,
                              IrmoIteratorFilter callback,
                              void *callback_data)
{
        iter->filter = callback;
        iter->filter_data = callback_data;
}


