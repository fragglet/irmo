
/*
 
Copyright (c) 2005, Simon Howard
All rights reserved.

Redistribution and use in source and binary forms, with or without 
modification, are permitted provided that the following conditions 
are met:

 * Redistributions of source code must retain the above copyright 
   notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright 
   notice, this list of conditions and the following disclaimer in 
   the documentation and/or other materials provided with the 
   distribution.
 * Neither the name of the C Algorithms project nor the names of its 
   contributors may be used to endorse or promote products derived 
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE 
COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT 
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdlib.h>
#include <string.h>

#include "arraylist.h"

/* Automatically resizing array */

IrmoArrayList *irmo_arraylist_new(int length)
{
	IrmoArrayList *new_arraylist;

	/* If the length is not specified, use a sensible default */
	
	if (length <= 0) {
		length = 16;
	}
	
	/* Allocate the new IrmoArrayList and fill in the fields.  There are 
	 * initially no entries. */

	new_arraylist = (IrmoArrayList *) malloc(sizeof(IrmoArrayList));
	new_arraylist->_alloced = length;
	new_arraylist->length = 0;

	/* Allocate the data array */

	new_arraylist->data = calloc(length, sizeof(void *));

	return new_arraylist;    
}

void irmo_arraylist_free(IrmoArrayList *arraylist)
{
	/* Do not free if a NULL pointer is passed */

	if (arraylist != NULL) {
		free(arraylist->data);
		free(arraylist);
	}
}

static void irmo_arraylist_enlarge(IrmoArrayList *arraylist)
{
	/* Double the allocated size */

	arraylist->_alloced *= 2;
	
	/* Reallocate the array to the new size */

	arraylist->data = realloc(arraylist->data, 
	                          sizeof(void *) * arraylist->_alloced);
}

int irmo_arraylist_insert(IrmoArrayList *arraylist, int index, void *data)
{
	/* Sanity check the index */

	if (index < 0 || index > arraylist->length) {
		return 0;
	}

	/* Increase the size if necessary */
	
	if (arraylist->length + 1 > arraylist->_alloced) {
		irmo_arraylist_enlarge(arraylist);
	}

	/* Move the contents of the array forward from the index
	 * onwards */

	memmove(&arraylist->data[index + 1], 
	        &arraylist->data[index],
	        (arraylist->length - index) * sizeof(void *));

	/* Insert the new entry at the index */

	arraylist->data[index] = data;
	++arraylist->length;

	return 1;
}

void irmo_arraylist_append(IrmoArrayList *arraylist, void *data)
{
	irmo_arraylist_insert(arraylist, arraylist->length, data);
}

void irmo_arraylist_prepend(IrmoArrayList *arraylist, void *data)
{
	irmo_arraylist_insert(arraylist, 0, data);
}

void irmo_arraylist_remove_range(IrmoArrayList *arraylist, int index, int length)
{
	/* Check this is a valid range */

	if (index < 0 || length < 0 || index + length > arraylist->length) {
		return;
	}

	/* Move back the entries following the range to be removed */

	memmove(&arraylist->data[index],
	        &arraylist->data[index + length],
	        (arraylist->length - (index + length)) * sizeof(void *));

	/* Decrease the counter */

	arraylist->length -= length;
}

void irmo_arraylist_remove(IrmoArrayList *arraylist, int index)
{
	irmo_arraylist_remove_range(arraylist, index, 1);
}

int irmo_arraylist_index_of(IrmoArrayList *arraylist, 
                       IrmoArrayListEqualFunc callback,
                       void *data)
{
	int i;

	for (i=0; i<arraylist->length; ++i) {
		if (callback(arraylist->data[i], data) != 0)
			return i;
	}

	return -1;
}

void irmo_arraylist_clear(IrmoArrayList *arraylist)
{
	/* To clear the list, simply set the length to zero */
	
	arraylist->length = 0;
}

static void irmo_arraylist_sort_internal(void **list_data, int list_length,
                                    IrmoArrayListCompareFunc compare_func)
{
	void *pivot;
	void *tmp;
	int i;
	int list1_length;
	int list2_length;

	/* If less than two items, it is always sorted. */

	if (list_length <= 1) {
		return;
	}

	/* Take the last item as the pivot. */

	pivot = list_data[list_length-1];

	/* Divide the list into two lists:
	 *
	 * List 1 contains data less than the pivot.
	 * List 2 contains data more than the pivot.
	 *
	 * As the lists are build up, they are stored sequentially after
	 * each other, ie. list_data[list1_length-1] is the last item
	 * in list 1, list_data[list1_length] is the first item in
	 * list 2.
	 */

	list1_length = 0;

	for (i=0; i<list_length-1; ++i) {

		if (compare_func(list_data[i], pivot) < 0) {

			/* This should be in list 1.  Therefore it is in the wrong
			 * position. Swap the data immediately following the last
			 * item in list 1 with this data. */

			tmp = list_data[i];
			list_data[i] = list_data[list1_length];
			list_data[list1_length] = tmp;

			++list1_length;

		} else {
			/* This should be in list 2.  This is already in the right
			 * position. */
		}
	}

	/* The length of list 2 can be calculated. */

	list2_length = list_length - list1_length - 1;

	/* list_data[0..list1_length-1] now contains all items which are
	 * before the pivot. 
	 * list_data[list1_length..list_length-2] contains all items after
	 * or equal to the pivot. */

	/* Move the pivot into place, by swapping it with the item 
	 * immediately following the end of list 1.  */

	list_data[list_length-1] = list_data[list1_length];
	list_data[list1_length] = pivot;
	
	/* Recursively sort the sublists. */

	irmo_arraylist_sort_internal(list_data, list1_length, compare_func);

	irmo_arraylist_sort_internal(&list_data[list1_length + 1], list2_length,
	                        compare_func);
}

void irmo_arraylist_sort(IrmoArrayList *arraylist, IrmoArrayListCompareFunc compare_func)
{
	/* Perform the recursive sort */
	
	irmo_arraylist_sort_internal(arraylist->data, arraylist->length, compare_func);
}

