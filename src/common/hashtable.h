
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
 * @file hashtable.h
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
 */

#ifndef IRMO_COMMON_HASHTABLE_H
#define IRMO_COMMON_HASHTABLE_H

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * A hash table structure.
 */

typedef struct _IrmoHashTable IrmoHashTable;

/**
 * Hash function used to generate hash values for keys used in a hash
 * table.
 *
 * @param data   The value to generate a hash value for.
 * @return       The hash value.
 */

typedef unsigned long (*IrmoHashTableHashFunc)(void *data);

/**
 * Function used to compare two keys for equality.
 *
 * @return   Non-zero if the two keys are equal, zero if the keys are 
 *           not equal.
 */

typedef int (*IrmoHashTableEqualFunc)(void *data1, void *data2);

/**
 * Type of function used to free keys and values when entries are
 * removed from a hash table.
 */

typedef void (*IrmoHashTableFreeFunc)(void *data);

/**
 * Type of function used as a callback when iterating over data.
 * See @ref irmo_hash_table_foreach.
 *
 * @param key            The key to the current element being iterated over.
 * @param value          The value of the current element being iterated over.
 * @param user_data      Extra data passed to the @ref irmo_hash_table_foreach
 *                       function.
 */

typedef void (*IrmoHashTableIterator)(void *key, void *value, void *user_data);

/**
 * Type of function used as a callback when iterating over a hash table,
 * selectively removing entries.
 * See @ref irmo_hash_table_foreach_remove.
 *
 * @param key            The key to the current element being iterated over.
 * @param value          The value of the current element being iterated over.
 * @param user_data      Extra data passed to the @ref irmo_hash_table_foreach
 *                       function.
 * @return               Non-zero (true) if the entry should be removed
 *                       from the hash table.  Zero (false) if the entry 
 *                       should not be removed from the hash table.
 */

typedef int (*IrmoHashTableRemoveIterator)(void *key, 
                                       void *value, 
                                       void *user_data);

/**
 * Create a new hash table.
 *
 * @param hash_func            Function used to generate hash keys for the 
 *                             keys used in the table.
 * @param equal_func           Function used to test keys used in the table 
 *                             for equality.
 * @return                     A new hash table structure.
 */

IrmoHashTable *irmo_hash_table_new(IrmoHashTableHashFunc hash_func, 
                          IrmoHashTableEqualFunc equal_func);

/**
 * Destroy a hash table.
 *
 * @param hashtable            The hash table to destroy.
 */

void irmo_hash_table_free(IrmoHashTable *hashtable);

/**
 * Register functions used to free the key and value when an entry is
 * removed from a hash table.
 *
 * @param hashtable            The hash table.
 * @param key_free_func        Function used to free keys.
 * @param value_free_func      Function used to free values.
 */

void irmo_hash_table_register_free_functions(IrmoHashTable *hashtable,
                                        IrmoHashTableFreeFunc key_free_func,
                                        IrmoHashTableFreeFunc value_free_func);

/**
 * Insert a value into a hash table, overwriting any existing entry 
 * using the same key.
 *
 * @param hashtable            The hash table.
 * @param key                  The key for the new value.
 * @param value                The value to insert.
 */

void irmo_hash_table_insert(IrmoHashTable *hashtable, void *key, void *value);

/**
 * Look up a value in a hash table by key.
 *
 * @param hashtable           The hash table.
 * @param key                 The key of the value to look up.
 * @return                    The value, or NULL if there is no value with
 *                            that key in the hash table.
 */

void *irmo_hash_table_lookup(IrmoHashTable *hashtable, void *key);

/**
 * Remove a value from a hash table.
 *
 * @param hashtable           The hash table.
 * @param key                 The key of the value to remove.
 * @return                    Non-zero if a key was removed, or zero if the
 *                            specified key was not found in the hash table.
 */

int irmo_hash_table_remove(IrmoHashTable *hashtable, void *key);

/** 
 * Retrieve the number of entries in a hash table.
 *
 * @param hashtable           The hash table.
 * @return                    The number of entries in the hash table.
 */

int irmo_hash_table_num_entries(IrmoHashTable *hashtable);

/**
 * Iterate over all key-value pairs in a hash table.
 *
 * @param hashtable           The hash table.
 * @param iterator            Callback function to invoke for each element.
 * @param user_data           Extra data to pass to the iterator function
 *                            as context.
 */

void irmo_hash_table_foreach(IrmoHashTable *hashtable, IrmoHashTableIterator iterator,
                        void *user_data);

/**
 * Iterate over all key-value pairs in a hash table, selectively
 * removing entries.
 *
 * @param hashtable           The hash table.
 * @param iterator            Callback function to invoke for each element.
 * @param user_data           Extra data to pass to the iterator function
 *                            as context.
 * @return                    The total number of entries removed from
 *                            the hash table.
 */

int irmo_hash_table_foreach_remove(IrmoHashTable *hashtable, 
                              IrmoHashTableRemoveIterator iterator,
                              void *user_data);

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_COMMON_HASHTABLE_H */

