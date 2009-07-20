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
 * @file hash-table.h
 *
 * @brief Hash table.
 *
 * A hash table stores a set of values which can be addressed by a 
 * key.  Given the key, the corresponding value can be looked up
 * quickly.
 *
 * To create a hash table, use @ref irmo_hash_table_new.  To destroy a 
 * hash table, use @ref irmo_hash_table_free.
 *
 * To insert a value into a hash table, use @ref irmo_hash_table_insert.
 *
 * To remove a value from a hash table, use @ref irmo_hash_table_remove.
 *
 * To look up a value by its key, use @ref irmo_hash_table_lookup.
 *
 * To iterate over all values in a hash table, use 
 * @ref irmo_hash_table_iterate to initialise a @ref IrmoHashTableIterator
 * structure.  Each value can then be read in turn using 
 * @ref irmo_hash_table_iter_next and @ref irmo_hash_table_iter_has_more.
 */

#ifndef IRMO_ALGO_HASH_TABLE_H
#define IRMO_ALGO_HASH_TABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * A hash table structure.
 */

typedef struct _IrmoHashTable IrmoHashTable;

/**
 * Structure used to iterate over a hash table.
 */

typedef struct _IrmoHashTableIterator IrmoHashTableIterator;

/**
 * Internal structure representing an entry in a hash table.
 */

typedef struct _IrmoHashTableEntry IrmoHashTableEntry;

/**
 * A key to look up a value in a @ref IrmoHashTable.
 */

typedef void *IrmoHashTableKey;

/**
 * A value stored in a @ref IrmoHashTable.
 */

typedef void *IrmoHashTableValue;

/**
 * Definition of a @ref IrmoHashTableIterator.
 */

struct _IrmoHashTableIterator {
	IrmoHashTable *hash_table;
	IrmoHashTableEntry *next_entry;
	unsigned int next_chain;
};

/**
 * A null @ref IrmoHashTableValue. 
 */

#define IRMO_HASH_TABLE_NULL ((void *) 0)

/**
 * Hash function used to generate hash values for keys used in a hash
 * table.
 *
 * @param value  The value to generate a hash value for.
 * @return       The hash value.
 */

typedef unsigned long (*IrmoHashTableHashFunc)(IrmoHashTableKey value);

/**
 * Function used to compare two keys for equality.
 *
 * @return   Non-zero if the two keys are equal, zero if the keys are 
 *           not equal.
 */

typedef int (*IrmoHashTableEqualFunc)(IrmoHashTableKey value1, IrmoHashTableKey value2);

/**
 * Type of function used to free keys when entries are removed from a 
 * hash table.
 */

typedef void (*IrmoHashTableKeyFreeFunc)(IrmoHashTableKey value);

/**
 * Type of function used to free values when entries are removed from a 
 * hash table.
 */

typedef void (*IrmoHashTableValueFreeFunc)(IrmoHashTableValue value);

/**
 * Create a new hash table.
 *
 * @param hash_func            Function used to generate hash keys for the 
 *                             keys used in the table.
 * @param equal_func           Function used to test keys used in the table 
 *                             for equality.
 * @return                     A new hash table structure, or NULL if it 
 *                             was not possible to allocate the new hash
 *                             table.
 */

IrmoHashTable *irmo_hash_table_new(IrmoHashTableHashFunc hash_func, 
                          IrmoHashTableEqualFunc equal_func);

/**
 * Destroy a hash table.
 *
 * @param hash_table           The hash table to destroy.
 */

void irmo_hash_table_free(IrmoHashTable *hash_table);

/**
 * Register functions used to free the key and value when an entry is
 * removed from a hash table.
 *
 * @param hash_table           The hash table.
 * @param key_free_func        Function used to free keys.
 * @param value_free_func      Function used to free values.
 */

void irmo_hash_table_register_free_functions(IrmoHashTable *hash_table,
                                        IrmoHashTableKeyFreeFunc key_free_func,
                                        IrmoHashTableValueFreeFunc value_free_func);

/**
 * Insert a value into a hash table, overwriting any existing entry 
 * using the same key.
 *
 * @param hash_table           The hash table.
 * @param key                  The key for the new value.
 * @param value                The value to insert.
 * @return                     Non-zero if the value was added successfully,
 *                             or zero if it was not possible to allocate
 *                             memory for the new entry.
 */

int irmo_hash_table_insert(IrmoHashTable *hash_table, 
                      IrmoHashTableKey key, 
                      IrmoHashTableValue value);

/**
 * Look up a value in a hash table by key.
 *
 * @param hash_table          The hash table.
 * @param key                 The key of the value to look up.
 * @return                    The value, or @ref IRMO_HASH_TABLE_NULL if there 
 *                            is no value with that key in the hash table.
 */

IrmoHashTableValue irmo_hash_table_lookup(IrmoHashTable *hash_table, 
                                 IrmoHashTableKey key);

/**
 * Remove a value from a hash table.
 *
 * @param hash_table          The hash table.
 * @param key                 The key of the value to remove.
 * @return                    Non-zero if a key was removed, or zero if the
 *                            specified key was not found in the hash table.
 */

int irmo_hash_table_remove(IrmoHashTable *hash_table, IrmoHashTableKey key);

/** 
 * Retrieve the number of entries in a hash table.
 *
 * @param hash_table          The hash table.
 * @return                    The number of entries in the hash table.
 */

unsigned int irmo_hash_table_num_entries(IrmoHashTable *hash_table);

/**
 * Initialise a @ref IrmoHashTableIterator to iterate over a hash table.
 *
 * @param hash_table          The hash table.
 * @param iter                Pointer to an iterator structure to 
 *                            initialise.
 */

void irmo_hash_table_iterate(IrmoHashTable *hash_table, IrmoHashTableIterator *iter);

/**
 * Determine if there are more keys in the hash table to iterate
 * over. 
 *
 * @param iterator            The hash table iterator.
 * @return                    Zero if there are no more values to iterate
 *                            over, non-zero if there are more values to 
 *                            iterate over.
 */

int irmo_hash_table_iter_has_more(IrmoHashTableIterator *iterator);

/**
 * Using a hash table iterator, retrieve the next key.
 *
 * @param iterator            The hash table iterator.
 * @return                    The next key from the hash table, or 
 *                            @ref IRMO_HASH_TABLE_NULL if there are no more 
 *                            keys to iterate over.
 */

IrmoHashTableValue irmo_hash_table_iter_next(IrmoHashTableIterator *iterator);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ALGO_HASH_TABLE_H */

