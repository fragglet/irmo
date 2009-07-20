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
 * @file arraylist.h
 *
 * @brief Automatically resizing array 
 *
 * IrmoArrayLists are arrays of pointers which automatically increase in 
 * size.
 *
 * To create an IrmoArrayList, use @ref irmo_arraylist_new.
 * To destroy an IrmoArrayList, use @ref irmo_arraylist_free.
 *
 * To add a value to an IrmoArrayList, use @ref irmo_arraylist_prepend, 
 * @ref irmo_arraylist_append, or @ref irmo_arraylist_insert.
 *
 * To remove a value from an IrmoArrayList, use @ref irmo_arraylist_remove
 * or @ref irmo_arraylist_remove_range.
 */

#ifndef IRMO_ALGO_ARRAYLIST_H
#define IRMO_ALGO_ARRAYLIST_H

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A value to be stored in an @ref IrmoArrayList.
 */

typedef void *IrmoArrayListValue;

/**
 * An IrmoArrayList structure.  New IrmoArrayLists can be created using the 
 * irmo_arraylist_new function.
 *
 * @see irmo_arraylist_new 
 */

typedef struct _IrmoArrayList IrmoArrayList;

/**
 * Definition of an @ref IrmoArrayList.
 */ 

struct _IrmoArrayList {

	/** Entries in the array */
	
	IrmoArrayListValue *data;

	/** Length of the array */
	
	unsigned int length;

	/** Private data and should not be accessed */

	unsigned int _alloced;
};

/**
 * Compare two values in an arraylist to determine if they are equal.
 *
 * @return Non-zero if the values are not equal, zero if they are equal.
 */

typedef int (*IrmoArrayListEqualFunc)(IrmoArrayListValue value1, IrmoArrayListValue value2);

/**
 * Compare two values in an arraylist.  Used by @ref irmo_arraylist_sort
 * when sorting values.
 *
 * @param value1              The first value.
 * @param value2              The second value.
 * @return                    A negative number if value1 should be sorted
 *                            before value2, a positive number if value2 should
 *                            be sorted before value1, zero if the two values
 *                            are equal.
 */

typedef int (*IrmoArrayListCompareFunc)(IrmoArrayListValue value1,
                                    IrmoArrayListValue value2);

/**
 * Allocate a new IrmoArrayList for use.
 *
 * @param length         Hint to the initialise function as to the amount
 *                       of memory to allocate initially to the IrmoArrayList.
 *                       If a value of zero is given, a sensible default
 *                       size is used.
 * @return               A new arraylist, or NULL if it was not possible
 *                       to allocate the memory.
 * @see irmo_arraylist_free
 */

IrmoArrayList *irmo_arraylist_new(unsigned int length);

/**
 * Destroy an IrmoArrayList and free back the memory it uses.
 *
 * @param arraylist      The IrmoArrayList to free.
 */

void irmo_arraylist_free(IrmoArrayList *arraylist);

/**
 * Append a value to the end of an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 * @param data           The value to append.
 * @return               Non-zero if the request was successful, zero
 *                       if it was not possible to allocate more memory
 *                       for the new entry.
 */

int irmo_arraylist_append(IrmoArrayList *arraylist, IrmoArrayListValue data);

/** 
 * Prepend a value to the beginning of an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 * @param data           The value to prepend.
 * @return               Non-zero if the request was successful, zero
 *                       if it was not possible to allocate more memory
 *                       for the new entry.
 */

int irmo_arraylist_prepend(IrmoArrayList *arraylist, IrmoArrayListValue data);

/**
 * Remove the entry at the specified location in an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 * @param index          The index of the entry to remove.
 */

void irmo_arraylist_remove(IrmoArrayList *arraylist, unsigned int index);

/**
 * Remove a range of entries at the specified location in an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 * @param index          The index of the start of the range to remove.
 * @param length         The length of the range to remove.
 */

void irmo_arraylist_remove_range(IrmoArrayList *arraylist, unsigned int index,
                            unsigned int length);

/**
 * Insert a value at the specified index in an IrmoArrayList.
 * The index where the new value can be inserted is limited by the 
 * size of the IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 * @param index          The index at which to insert the value.
 * @param data           The value.
 * @return               Returns zero if unsuccessful, else non-zero 
 *                       if successful (due to an invalid index or 
 *                       if it was impossible to allocate more memory).
 */

int irmo_arraylist_insert(IrmoArrayList *arraylist, unsigned int index,
                     IrmoArrayListValue data);

/**
 * Find the index of a particular value in an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList to search.
 * @param callback       Callback function to be invoked to compare
 *                       values in the list with the value to be
 *                       searched for.
 * @param data           The value to search for.
 * @return               The index of the value if found, or -1 if not found.
 */

int irmo_arraylist_index_of(IrmoArrayList *arraylist,
                       IrmoArrayListEqualFunc callback,
                       IrmoArrayListValue data);

/** 
 * Remove all entries from an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 */

void irmo_arraylist_clear(IrmoArrayList *arraylist);

/** 
 * Sort the values in an IrmoArrayList.
 *
 * @param arraylist      The IrmoArrayList.
 * @param compare_func   Function used to compare values in sorting.
 */

void irmo_arraylist_sort(IrmoArrayList *arraylist, IrmoArrayListCompareFunc compare_func);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ALGO_ARRAYLIST_H */

