/*

Copyright (c) 2005-2008, Simon Howard

Permission to use, copy, modify, and/or distribute this software 
for any purpose with or without fee is hereby granted, provided 
that the above copyright notice and this permission notice appear 
in all copies. 

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL 
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED 
WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE 
AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR 
CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM 
LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, 
NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN      
CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 

 */

#include <stdlib.h>

#include "slist.h"

/* malloc() / free() testing */

#ifdef ALLOC_TESTING
#include "alloc-testing.h"
#endif

/* A singly-linked list */

struct _IrmoSListEntry {
	IrmoSListValue data;
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

IrmoSListEntry *irmo_slist_prepend(IrmoSListEntry **list, IrmoSListValue data)
{
	IrmoSListEntry *newentry;

	/* Create new entry */

	newentry = malloc(sizeof(IrmoSListEntry));

	if (newentry == NULL) {
		return NULL;
	}
	
	newentry->data = data;

	/* Hook into the list start */

	newentry->next = *list;
	*list = newentry;

	return newentry;
}

IrmoSListEntry *irmo_slist_append(IrmoSListEntry **list, IrmoSListValue data)
{
	IrmoSListEntry *rover;
	IrmoSListEntry *newentry;

	/* Create new list entry */

	newentry = malloc(sizeof(IrmoSListEntry));

	if (newentry == NULL) {
		return NULL;
	}
	
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

IrmoSListValue irmo_slist_data(IrmoSListEntry *listentry)
{
	return listentry->data;
}

IrmoSListEntry *irmo_slist_next(IrmoSListEntry *listentry)
{
	return listentry->next;
}

IrmoSListEntry *irmo_slist_nth_entry(IrmoSListEntry *list, unsigned int n)
{
	IrmoSListEntry *entry;
	unsigned int i;

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

IrmoSListValue irmo_slist_nth_data(IrmoSListEntry *list, unsigned int n)
{
	IrmoSListEntry *entry;

	/* Find the specified entry */

	entry = irmo_slist_nth_entry(list, n);

	/* If out of range, return NULL, otherwise return the data */

	if (entry == NULL) {
		return IRMO_SLIST_NULL;
	} else {
		return entry->data;
	}
}

unsigned int irmo_slist_length(IrmoSListEntry *list)
{
	IrmoSListEntry *entry;
	unsigned int length;

	length = 0;
	entry = list;

	while (entry != NULL) {

		/* Count the number of entries */

		++length;

		entry = entry->next;
	}

	return length;
}

IrmoSListValue *irmo_slist_to_array(IrmoSListEntry *list)
{
	IrmoSListEntry *rover;
	IrmoSListValue *array;
	unsigned int length;
	unsigned int i;

	/* Allocate an array equal in size to the list length */

	length = irmo_slist_length(list);

	array = malloc(sizeof(IrmoSListValue) * length);

	if (array == NULL) {
		return NULL;
	}

	/* Add all entries to the array */

	rover = list;

	for (i=0; i<length; ++i) {

		/* Add this node's data */

		array[i] = rover->data;

		/* Jump to the next list node */

		rover = rover->next;
	}

	return array;
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

unsigned int irmo_slist_remove_data(IrmoSListEntry **list, IrmoSListEqualFunc callback,
                               IrmoSListValue data)
{
	IrmoSListEntry **rover;
	IrmoSListEntry *next;
	unsigned int entries_removed;

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
                            IrmoSListValue data)
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

void irmo_slist_iterate(IrmoSListEntry **list, IrmoSListIterator *iter)
{
	/* Start iterating from the beginning of the list. */

	iter->prev_next = list;

	/* We have not yet read the first item. */

	iter->current = NULL;
}

int irmo_slist_iter_has_more(IrmoSListIterator *iter)
{
	if (iter->current == NULL || iter->current != *iter->prev_next) {

		/* Either we have not read the first entry, the current
		 * item was removed or we have reached the end of the
		 * list.  Use prev_next to determine if we have a next
		 * value to iterate over. */

		return *iter->prev_next != NULL;

	} else {
	
		/* The current entry has not been deleted.  There
		 * is a next entry if current->next is not NULL. */

		return iter->current->next != NULL;
	}
}

IrmoSListValue irmo_slist_iter_next(IrmoSListIterator *iter)
{
	if (iter->current == NULL || iter->current != *iter->prev_next) {

		/* Either we are reading the first entry, we have reached
		 * the end of the list, or the previous entry was removed.
		 * Get the next entry with iter->prev_next. */

		iter->current = *iter->prev_next;

	} else {

		/* Last value returned from irmo_slist_iter_next was not
		 * deleted. Advance to the next entry. */

		iter->prev_next = &iter->current->next;
		iter->current = iter->current->next;
	}

	/* Have we reached the end of the list? */

	if (iter->current == NULL) {
		return IRMO_SLIST_NULL;
	} else {
		return iter->current->data;
	}
}

void irmo_slist_iter_remove(IrmoSListIterator *iter)
{
	if (iter->current == NULL || iter->current != *iter->prev_next) {
		
		/* Either we have not yet read the first item, we have 
		 * reached the end of the list, or we have already removed
		 * the current value.  Either way, do nothing. */
	
	} else {
		
		/* Remove the current entry */

		*iter->prev_next = iter->current->next;
		free(iter->current);
		iter->current = NULL;
	}
}

