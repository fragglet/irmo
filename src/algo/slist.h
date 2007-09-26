
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
 * To add new data at the start of a list, use @ref irmo_slist_prepend.
 * To add new data at the end of a list, use @ref irmo_slist_append.
 *
 * To find the length of a list, use @ref irmo_slist_length.
 *
 * To access data in a list by its index in the list, use 
 * @ref irmo_slist_nth_data.
 *
 * To search a list for data, use @ref irmo_slist_find_data.
 *
 * To sort a list into an order, use @ref irmo_slist_sort.
 *
 * To iterate over a list, use @ref irmo_slist_foreach.
 *
 * To find a particular entry in a list by its index, use 
 * @ref irmo_slist_nth_entry.
 *
 * Given a particular entry in a list:
 *
 * @li To find the next entry, use @ref irmo_slist_next.
 * @li To access the data stored at the entry, use @ref irmo_slist_data.
 * @li To remove the entry, use @ref irmo_slist_remove_entry.
 *
 */

#ifndef IRMO_ALGO_SLIST_H
#define IRMO_ALGO_SLIST_H

/**
 * Represents an entry in a singly-linked list.  The empty list is
 * represented by a NULL pointer. To initialise a new singly linked 
 * list, simply create a variable of this type 
 * containing a pointer to NULL.
 */

typedef struct _IrmoSListEntry IrmoSListEntry;

/**
 * Callback function used for iterating over a list
 */

typedef void (*IrmoSListIterator)(void *data, void *user_data);

/**
 * Callback function used to compare values in a list when sorting.
 *
 * @return   A negative value if data1 should be sorted before data2, 
 *           a positive value if data1 should be sorted after data2, 
 *           zero if data1 and data2 are equal.
 */

typedef int (*IrmoSListCompareFunc)(void *data1, void *data2);

/**
 * Callback function used to determine of two values in a list are
 * equal.
 *
 * @return   A non-zero value if data1 and data2 are equal, zero if they
 *           are not equal.
 */

typedef int (*IrmoSListEqualFunc)(void *data1, void *data2);

/**
 * Free an entire list.
 *
 * @param list           The list to free.
 */

void irmo_slist_free(IrmoSListEntry *list);

/**
 * Prepend data to the start of a list.
 *
 * @param list      Pointer to the list to prepend to.
 * @param data      Data to prepend.
 * @return          The new entry in the list.
 */

IrmoSListEntry *irmo_slist_prepend(IrmoSListEntry **list, void *data);

/**
 * Append data to the end of a list.
 *
 * @param list      Pointer to the list to append to.
 * @param data      Data to append.
 * @return          The new entry in the list.
 */

IrmoSListEntry *irmo_slist_append(IrmoSListEntry **list, void *data);

/** 
 * Retrieve the next entry in a list.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The next entry in the list.
 */

IrmoSListEntry *irmo_slist_next(IrmoSListEntry *listentry);

/**
 * Retrieve the data at a list entry.
 *
 * @param listentry    Pointer to the list entry.
 * @return             The data at the list entry.
 */

void *irmo_slist_data(IrmoSListEntry *listentry);

/** 
 * Retrieve the entry at a specified index in a list.
 *
 * @param list       The list.
 * @param n          The index into the list .
 * @return           The entry at the specified index, or NULL if out of range.
 */

IrmoSListEntry *irmo_slist_nth_entry(IrmoSListEntry *list, int n);

/** 
 * Retrieve the data at a specified entry in the list.
 *
 * @param list       The list.
 * @param n          The index into the list .
 * @return           The data at the specified index, or NULL if 
 *                   unsuccessful.
 */

void *irmo_slist_nth_data(IrmoSListEntry *list, int n);

/** 
 * Find the length of a list.
 *
 * @param list       The list.
 * @return           The number of entries in the list.
 */

int irmo_slist_length(IrmoSListEntry *list);

/**
 * Create a C array containing the contents of a list.
 *
 * @param list       The list.
 * @return           A newly-allocated C array containing all values in the
 *                   list.  The length of the array is equal to the length
 *                   of the list (see @ref irmo_slist_length).
 */

void **irmo_slist_to_array(IrmoSListEntry *list);

/** 
 * Iterate over all entries in a list.
 *
 * @param list       The list.
 * @param callback   Callback function to invoke for each entry in the list.
 * @param user_data  Extra data to pass to the callback function.
 */

void irmo_slist_foreach(IrmoSListEntry *list, IrmoSListIterator callback, void *user_data);

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
 * Remove all occurrences of a particular piece of data from a list.
 *
 * @param list       Pointer to the list.
 * @param callback   Callback function to invoke to compare data in the 
 *                   list with the data to remove.
 * @param data       The data to remove from the list.
 * @return           The number of entries removed from the list.
 */

int irmo_slist_remove_data(IrmoSListEntry **list, IrmoSListEqualFunc callback, void *data);

/**
 * Sort a list.
 *
 * @param list          Pointer to the list to sort.
 * @param compare_func  Function used to compare values in the list.
 */

void irmo_slist_sort(IrmoSListEntry **list, IrmoSListCompareFunc compare_func);

/**
 * Find the entry for a particular data item in a list.
 *
 * @param list           The list to search.
 * @param callback       Callback function to be invoked to determine if
 *                       values are equal to the data to search for.
 * @param data           The data to search for.
 * @return               The list entry of the item being searched for, or
 *                       NULL if not found.
 */

IrmoSListEntry *irmo_slist_find_data(IrmoSListEntry *list, 
                            IrmoSListEqualFunc callback,
                            void *data);

#endif /* #ifndef IRMO_ALGO_SLIST_H */

