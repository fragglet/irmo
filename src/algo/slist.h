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

/**
 * @file slist.h
 *
 * Singly-linked list.
 *
 * A singly-linked list stores a collection of values.  Each 
 * entry in the list (represented by a pointer to a @ref IrmoSListEntry
 * structure) contains a link to the next entry.  It is only 
 * possible to iterate over entries in a singly linked list in one 
 * direction.
 *
 * To create a new singly-linked list, create a variable which is
 * a pointer to a @ref IrmoSListEntry, and initialise it to NULL.
 *
 * To destroy a singly linked list, use @ref irmo_slist_free.
 *
 * To add a new value at the start of a list, use @ref irmo_slist_prepend.
 * To add a new value at the end of a list, use @ref irmo_slist_append.
 *
 * To find the length of a list, use @ref irmo_slist_length.
 *
 * To access a value in a list by its index in the list, use 
 * @ref irmo_slist_nth_data.
 *
 * To search a list for a value, use @ref irmo_slist_find_data.
 *
 * To sort a list into an order, use @ref irmo_slist_sort.
 *
 * To find a particular entry in a list by its index, use 
 * @ref irmo_slist_nth_entry.
 *
 * To iterate over each value in a list, use @ref irmo_slist_iterate to 
 * initialise a @ref IrmoSListIterator structure, with @ref irmo_slist_iter_next
 * and @ref irmo_slist_iter_has_more to retrieve each value in turn.
 * @ref irmo_slist_iter_remove can be used to efficiently remove the 
 * current entry from the list.
 *
 * Given a particular entry in a list (@ref IrmoSListEntry):
 *
 * @li To find the next entry, use @ref irmo_slist_next.
 * @li To access the value stored at the entry, use @ref irmo_slist_data.
 * @li To remove the entry, use @ref irmo_slist_remove_entry.
 *
 */

#ifndef IRMO_ALGO_SLIST_H
#define IRMO_ALGO_SLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Represents an entry in a singly-linked list.  The empty list is
 * represented by a NULL pointer. To initialise a new singly linked 
 * list, simply create a variable of this type 
 * containing a pointer to NULL.
 */

typedef struct _IrmoSListEntry IrmoSListEntry;

/**
 * Structure used to iterate over a list.
 */

typedef struct _IrmoSListIterator IrmoSListIterator;

/**
 * Value stored in a list.
 */

typedef void *IrmoSListValue;

/**
 * Definition of a @ref IrmoSListIterator.
 */

struct _IrmoSListIterator {
	IrmoSListEntry **prev_next;
	IrmoSListEntry *current;
};

/**
 * A null @ref IrmoSListValue.
 */

#define IRMO_SLIST_NULL ((void *) 0)

/**
 * Callback function used to compare values in a list when sorting.
 *
 * @return   A negative value if value1 should be sorted before value2, 
 *           a positive value if value1 should be sorted after value2, 
 *           zero if value1 and value2 are equal.
 */

typedef int (*IrmoSListCompareFunc)(IrmoSListValue value1, IrmoSListValue value2);

/**
 * Callback function used to determine of two values in a list are
 * equal.
 *
 * @return   A non-zero value if value1 and value2 are equal, zero if they
 *           are not equal.
 */

typedef int (*IrmoSListEqualFunc)(IrmoSListValue value1, IrmoSListValue value2);

/**
 * Free an entire list.
 *
 * @param list           The list to free.
 */

void irmo_slist_free(IrmoSListEntry *list);

/**
 * Prepend a value to the start of a list.
 *
 * @param list      Pointer to the list to prepend to.
 * @param data      The value to prepend.
 * @return          The new entry in the list, or NULL if it was not possible
 *                  to allocate a new entry.
 */

IrmoSListEntry *irmo_slist_prepend(IrmoSListEntry **list, IrmoSListValue data);

/**
 * Append a value to the end of a list.
 *
 * @param list      Pointer to the list to append to.
 * @param data      The value to append.
 * @return          The new entry in the list, or NULL if it was not possible
 *                  to allocate a new entry.
 */

IrmoSListEntry *irmo_slist_append(IrmoSListEntry **list, IrmoSListValue data);

/** 
 * Retrieve the next entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The next entry in the list.
 */

IrmoSListEntry *irmo_slist_next(IrmoSListEntry *listentry);

/**
 * Retrieve the value stored at a list entry.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The value at the list entry.
 */

IrmoSListValue irmo_slist_data(IrmoSListEntry *listentry);

/** 
 * Retrieve the entry at a specified index in a list.
 *
 * @param list       The list.
 * @param n          The index into the list .
 * @return           The entry at the specified index, or NULL if out of range.
 */

IrmoSListEntry *irmo_slist_nth_entry(IrmoSListEntry *list, unsigned int n);

/** 
 * Retrieve the value stored at a specified index in the list.
 *
 * @param list       The list.
 * @param n          The index into the list.
 * @return           The value stored at the specified index, or
 *                   @ref IRMO_SLIST_NULL if unsuccessful.
 */

IrmoSListValue irmo_slist_nth_data(IrmoSListEntry *list, unsigned int n);

/** 
 * Find the length of a list.
 *
 * @param list       The list.
 * @return           The number of entries in the list.
 */

unsigned int irmo_slist_length(IrmoSListEntry *list);

/**
 * Create a C array containing the contents of a list.
 *
 * @param list       The list.
 * @return           A newly-allocated C array containing all values in the
 *                   list, or NULL if it was not possible to allocate the 
 *                   memory for the array.  The length of the array is 
 *                   equal to the length of the list (see @ref irmo_slist_length).
 */

IrmoSListValue *irmo_slist_to_array(IrmoSListEntry *list);

/**
 * Remove an entry from a list.
 *
 * @param list       Pointer to the list.
 * @param entry      The list entry to remove.
 * @return           If the entry is not found in the list, returns zero,
 *                   else returns non-zero.
 */

int irmo_slist_remove_entry(IrmoSListEntry **list, IrmoSListEntry *entry);

/**
 * Remove all occurrences of a particular value from a list.
 *
 * @param list       Pointer to the list.
 * @param callback   Callback function to invoke to compare values in the
 *                   list with the value to remove.
 * @param data       The value to remove from the list.
 * @return           The number of entries removed from the list.
 */

unsigned int irmo_slist_remove_data(IrmoSListEntry **list,
                               IrmoSListEqualFunc callback,
                               IrmoSListValue data);

/**
 * Sort a list.
 *
 * @param list          Pointer to the list to sort.
 * @param compare_func  Function used to compare values in the list.
 */

void irmo_slist_sort(IrmoSListEntry **list, IrmoSListCompareFunc compare_func);

/**
 * Find the entry for a particular value in a list.
 *
 * @param list           The list to search.
 * @param callback       Callback function to be invoked to determine if
 *                       values in the list are equal to the value to be
 *                       searched for.
 * @param data           The value to search for.
 * @return               The list entry of the value being searched for, or
 *                       NULL if not found.
 */

IrmoSListEntry *irmo_slist_find_data(IrmoSListEntry *list, 
                            IrmoSListEqualFunc callback,
                            IrmoSListValue data);

/** 
 * Initialise a @ref IrmoSListIterator structure to iterate over a list.
 *
 * @param list           Pointer to the list to iterate over.
 * @param iter           Pointer to a @ref IrmoSListIterator structure to
 *                       initialise.
 */

void irmo_slist_iterate(IrmoSListEntry **list, IrmoSListIterator *iter);

/**
 * Determine if there are more values in the list to iterate over.
 *
 * @param iterator       The list iterator.
 * @return               Zero if there are no more values in the list to
 *                       iterate over, non-zero if there are more values to
 *                       read.
 */

int irmo_slist_iter_has_more(IrmoSListIterator *iterator);

/**
 * Using a list iterator, retrieve the next value from the list. 
 *
 * @param iterator       The list iterator.
 * @return               The next value from the list, or IRMO_SLIST_NULL if 
 *                       there are no more values in the list.
 */
	
IrmoSListValue irmo_slist_iter_next(IrmoSListIterator *iterator);

/** 
 * Delete the current entry in the list (the value last returned from
 * @ref irmo_slist_iter_next)
 *
 * @param iterator       The list iterator.
 */

void irmo_slist_iter_remove(IrmoSListIterator *iterator);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ALGO_SLIST_H */

