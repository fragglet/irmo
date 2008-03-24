//
// Copyright (C) 2007-8 Simon Howard
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "base/iterator.h"

#define NUM_TEST_VALUES 8

int test_values[] = { 4, 8, 15, 16, 23, 42, 9, 3 };

// Get an array of pointers to ints to iterate over.

void **get_pointer_array(void)
{
        static void *pvalues[NUM_TEST_VALUES];
        int i;

        for (i=0; i<NUM_TEST_VALUES; ++i) {
                pvalues[i] = &test_values[i];
        }
        
        return pvalues;
}

void test_array_iterator(void)
{
        IrmoIterator *iter;
        int i;
        int *val;

        iter = irmo_iterate_array(get_pointer_array(), NUM_TEST_VALUES);

        i = 0;

        while (irmo_iterator_has_more(iter)) {
                val = irmo_iterator_next(iter);

                assert(*val == test_values[i]);
                ++i;
        }

        assert(i == NUM_TEST_VALUES);

        irmo_iterator_free(iter);
}

void test_hash_table_iterator(void)
{
        IrmoHashTable *hashtable;
        IrmoIterator *iter;
        int found[100];
        char *val;
        int i;

        // Fill up a hash table
  
        hashtable = irmo_hash_table_new(irmo_string_hash, irmo_string_equal);

        for (i=0; i<100; ++i) {
                val = malloc(32);
                sprintf(val, "%i", i);
                irmo_hash_table_insert(hashtable, val, val);
        }

        // Iterate over the hash table and check for all values

        memset(found, 0, sizeof(found));

        iter = irmo_iterate_hash_table(hashtable);

        while (irmo_iterator_has_more(iter)) {
                val = irmo_iterator_next(iter);

                i = atoi(val);

                assert(found[i] == 0);
                found[i] = 1;
        }

        irmo_iterator_free(iter);

        // Check we found them all

        for (i=0; i<100; ++i) {
                assert(found[i] == 1);
        }
}

// Filter to make an iterator return only even values

int even_value_filter(void *_val, void *extra_data)
{
        int *val = _val;

        return (*val % 2) == 0;
}

void test_filter_1(void)
{
        IrmoIterator *iter;
        int i;
        int actual_even;
        int counted_even;
        int *val;

        // How many even values are in the test array?

        actual_even = 0;
        for (i=0; i<NUM_TEST_VALUES; ++i) {
                if ((test_values[i] % 2) == 0) {
                        ++actual_even;
                }
        }

        // Now iterate with a filter

        iter = irmo_iterate_array(get_pointer_array(), NUM_TEST_VALUES);

        irmo_iterator_set_filter(iter, even_value_filter, NULL);

        counted_even = 0;
        while (irmo_iterator_has_more(iter)) {
                val = irmo_iterator_next(iter);
                assert((*val % 2) == 0);
                ++counted_even;
        }

        irmo_iterator_free(iter);

        assert(counted_even == actual_even);
}

// Filter all values (iterator never returns anything)

int filter_never(void *value, void *extra_data)
{
        return 0;
}

void test_filter_2(void)
{
        IrmoIterator *iter;

        iter = irmo_iterate_array(get_pointer_array(), NUM_TEST_VALUES);
        irmo_iterator_set_filter(iter, filter_never, NULL);

        assert(!irmo_iterator_has_more(iter));
        assert(irmo_iterator_next(iter) == NULL);

        irmo_iterator_free(iter);
}

int main(int argc, char *argv[])
{
        test_array_iterator();
        test_hash_table_iterator();
        test_filter_1();
        test_filter_2();

        return 0;
}

