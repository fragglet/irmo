
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

#include "slist.h"

/* A singly-linked list */

struct _IrmoSListEntry {
	void *data;
	IrmoSListEntry *next;
};

void irmo_slist_free(IrmoSListEntry *list)
{
	IrmoSListEntry *entry;

	/* Iterate over each entry, freeing each list entry, until the
	 * end is reached */

	entry = list;

	while (entry != NULL) {
		IrmoSListEntry *next;
	   
		next = entry->next;

		free(entry);

		entry = next;
	}
}

IrmoSListEntry *irmo_slist_prepend(IrmoSListEntry **list, void *data)
{
	IrmoSListEntry *newentry;

	/* Create new entry */

	newentry = malloc(sizeof(IrmoSListEntry));
	newentry->data = data;

	/* Hook into the list start */

	newentry->next = *list;
	*list = newentry;

	return newentry;
}

IrmoSListEntry *irmo_slist_append(IrmoSListEntry **list, void *data)
{
	IrmoSListEntry *rover;
	IrmoSListEntry *newentry;

	/* Create new list entry */

	newentry = malloc(sizeof(IrmoSListEntry));
	newentry->data = data;
	newentry->next = NULL;
	
	/* Hooking into the list is different if the list is empty */

	if (*list == NULL) {

		/* Create the start of the list */

		*list = newentry;

	} else {

		/* Find the end of list */

		for (rover=*list; rover->next != NULL; rover = rover->next);

		/* Add to the end of list */

		rover->next = newentry;
	}

	return newentry;
}

void *irmo_slist_data(IrmoSListEntry *listentry)
{
	return listentry->data;
}

IrmoSListEntry *irmo_slist_next(IrmoSListEntry *listentry)
{
	return listentry->next;
}

IrmoSListEntry *irmo_slist_nth_entry(IrmoSListEntry *list, int n)
{
	IrmoSListEntry *entry;
	int i;

	/* Negative values are always out of range */

	if (n < 0) {
		return NULL;
	}

	/* Iterate through n list entries to reach the desired entry.
	 * Make sure we do not reach the end of the list. */

	entry = list;

	for (i=0; i<n; ++i) {

		if (entry == NULL) {
			return NULL;
		}
		entry = entry->next;
	}

	return entry;
}

void *irmo_slist_nth_data(IrmoSListEntry *list, int n)
{
	IrmoSListEntry *entry;

	/* Find the specified entry */

	entry = irmo_slist_nth_entry(list, n);

	/* If out of range, return NULL, otherwise return the data */

	if (entry == NULL) {
		return NULL;
	} else {
		return entry->data;
	}
}

int irmo_slist_length(IrmoSListEntry *list)
{
	IrmoSListEntry *entry;
	int length;

	length = 0;
	entry = list;

	while (entry != NULL) {
		
		/* Count the number of entries */
		
		++length;

		entry = entry->next;
	}

	return length;
}

void **irmo_slist_to_array(IrmoSListEntry *list)
{
	IrmoSListEntry *rover;
	int listlen;
	void **array;
	int i;

	/* Allocate an array equal in size to the list length */
	
	listlen = irmo_slist_length(list);

	array = calloc(sizeof(void *), listlen);
	
	/* Add all entries to the array */
	
	rover = list;
	
	for (i=0; i<listlen; ++i) {

		/* Add this node's data */

		array[i] = rover->data;
		
		/* Jump to the next list node */

		rover = rover->next;
	}

	return array;
}


void irmo_slist_foreach(IrmoSListEntry *list, IrmoSListIterator callback, void *user_data)
{
	IrmoSListEntry *entry;

	/* Iterate over each entry in the list */

	entry = list;

	while (entry != NULL) {

		/* Invoke the callback function */

		callback(entry->data, user_data);

		/* Advance to the next entry */

		entry = entry->next;
	}
}

int irmo_slist_remove_entry(IrmoSListEntry **list, IrmoSListEntry *entry)
{
	IrmoSListEntry *rover;

	/* If the list is empty, or entry is NULL, always fail */

	if (*list == NULL || entry == NULL) {
		return 0;
	}
	
	/* Action to take is different if the entry is the first in the list */

	if (*list == entry) {

		/* Unlink the first entry and update the starting pointer */

		*list = entry->next;

	} else {

		/* Search through the list to find the preceding entry */

		rover = *list;

		while (rover != NULL && rover->next != entry) {
			rover = rover->next;
		}

		if (rover == NULL) {

			/* Not found in list */

			return 0;

		} else {

			/* rover->next now points at entry, so rover is the preceding
			 * entry. Unlink the entry from the list. */

			rover->next = entry->next;
		}
	}

	/* Free the list entry */

	free(entry);

	/* Operation successful */

	return 1;
}

int irmo_slist_remove_data(IrmoSListEntry **list, IrmoSListEqualFunc callback, void *data)
{
	IrmoSListEntry **rover;
	IrmoSListEntry *next;
	int entries_removed;

	entries_removed = 0;

	/* Iterate over the list.  'rover' points at the entrypoint into the
	 * current entry, ie. the list variable for the first entry in the 
	 * list, or the "next" field of the preceding entry. */
	
	rover = list;

	while (*rover != NULL) {
		
		/* Should this entry be removed? */
		
		if (callback((*rover)->data, data) != 0) {
			
			/* Data found, so remove this entry and free */

			next = (*rover)->next;
			free(*rover);
			*rover = next;
			
			/* Count the number of entries removed */

			++entries_removed;
		} else {
			
			/* Advance to the next entry */

			rover = &((*rover)->next);
		}
	}

	return entries_removed;
}

/* Function used internally for sorting.  Returns the last entry in the
 * new sorted list */

static IrmoSListEntry *irmo_slist_sort_internal(IrmoSListEntry **list, 
                                       IrmoSListCompareFunc compare_func)
{
	IrmoSListEntry *pivot;
	IrmoSListEntry *rover;
	IrmoSListEntry *less_list, *more_list;
	IrmoSListEntry *less_list_end, *more_list_end;
	
	/* If there are less than two entries in this list, it is
	 * already sorted */

	if (*list == NULL || (*list)->next == NULL) {
		return *list;
	}

	/* The first entry is the pivot */

	pivot = *list;

	/* Iterate over the list, starting from the second entry.  Sort
	 * all entries into the less and more lists based on comparisons
	 * with the pivot */

	less_list = NULL;
	more_list = NULL;
	rover = (*list)->next;

	while (rover != NULL) {
		IrmoSListEntry *next = rover->next;

		if (compare_func(rover->data, pivot->data) < 0) {

			/* Place this in the less list */

			rover->next = less_list;
			less_list = rover;

		} else {

			/* Place this in the more list */

			rover->next = more_list;
			more_list = rover;

		}

		rover = next;
	}

	/* Sort the sublists recursively */

	less_list_end = irmo_slist_sort_internal(&less_list, compare_func);
	more_list_end = irmo_slist_sort_internal(&more_list, compare_func);

	/* Create the new list starting from the less list */

	*list = less_list;

	/* Append the pivot to the end of the less list.  If the less list
	 * was empty, start from the pivot */

	if (less_list == NULL) {
		*list = pivot;
	} else {
		less_list_end->next = pivot;
	}

	/* Append the more list after the pivot */

	pivot->next = more_list;

	/* Work out what the last entry in the list is.  If the more list was 
	 * empty, the pivot was the last entry.  Otherwise, the end of the 
	 * more list is the end of the total list. */

	if (more_list == NULL) {
		return pivot;
	} else {
		return more_list_end;
	}
}

void irmo_slist_sort(IrmoSListEntry **list, IrmoSListCompareFunc compare_func)
{
	irmo_slist_sort_internal(list, compare_func);
}

IrmoSListEntry *irmo_slist_find_data(IrmoSListEntry *list,
                            IrmoSListEqualFunc callback,
                            void *data)
{
	IrmoSListEntry *rover;

	/* Iterate over entries in the list until the data is found */

	for (rover=list; rover != NULL; rover=rover->next) {
		if (callback(rover->data, data) != 0) {
			return rover;
		}
	}
	
	/* Not found */

	return NULL;
}

		
