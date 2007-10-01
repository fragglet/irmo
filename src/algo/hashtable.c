
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

/* Hash table implementation */

#include <stdlib.h>
#include <string.h>

#include "hashtable.h"

typedef struct _IrmoHashTableEntry IrmoHashTableEntry;

struct _IrmoHashTableEntry {
	IrmoHashTableKey key;
	IrmoHashTableValue value;
	IrmoHashTableEntry *next;
};

struct _IrmoHashTable {
	IrmoHashTableEntry **table;
	int table_size;
	IrmoHashTableHashFunc hash_func;
	IrmoHashTableEqualFunc equal_func;
	IrmoHashTableKeyFreeFunc key_free_func;
	IrmoHashTableValueFreeFunc value_free_func;
	int entries;
	int prime_index;
};

struct _IrmoHashTableIterator {
	IrmoHashTable *hashtable;
	IrmoHashTableEntry *current_entry;
	IrmoHashTableEntry *next_entry;
	int next_chain;
};

/* This is a set of good hash table prime numbers, from:
 *   http://planetmath.org/encyclopedia/GoodIrmoHashTablePrimes.html
 * Each prime is roughly double the previous value, and as far as
 * possible from the nearest powers of two. */

static const unsigned int irmo_hash_table_primes[] = {
	193, 389, 769, 1543, 3079, 6151, 12289, 24593, 49157, 98317,
	196613, 393241, 786433, 1572869, 3145739, 6291469,
	12582917, 25165843, 50331653, 100663319, 201326611,
	402653189, 805306457, 1610612741,
};

static const int irmo_hash_table_num_primes 
	= sizeof(irmo_hash_table_primes) / sizeof(int);

/* Internal function used to allocate the table on hashtable creation
 * and when enlarging the table */

static int irmo_hash_table_allocate_table(IrmoHashTable *hashtable)
{
	/* Determine the table size based on the current prime index.  
	 * An attempt is made here to ensure sensible behavior if the
	 * maximum prime is exceeded, but in practice other things are
	 * likely to break long before that happens. */

	if (hashtable->prime_index < irmo_hash_table_num_primes) {
		hashtable->table_size = irmo_hash_table_primes[hashtable->prime_index];
	} else {
		hashtable->table_size = hashtable->entries * 10;
	}

	/* Allocate the table and initialise to NULL for all entries */

	hashtable->table = calloc(hashtable->table_size, 
	                          sizeof(IrmoHashTableEntry *));

	return hashtable->table != NULL;
}

/* Free an entry, calling the free functions if there are any registered */

static void irmo_hash_table_free_entry(IrmoHashTable *hashtable, IrmoHashTableEntry *entry)
{
	/* If there is a function registered for freeing keys, use it to free
	 * the key */
	
	if (hashtable->key_free_func != NULL) {
		hashtable->key_free_func(entry->key);
	}

	/* Likewise with the value */

	if (hashtable->value_free_func != NULL) {
		hashtable->value_free_func(entry->value);
	}

	/* Free the data structure */
	
	free(entry);
}

IrmoHashTable *irmo_hash_table_new(IrmoHashTableHashFunc hash_func, 
                          IrmoHashTableEqualFunc equal_func)
{
	IrmoHashTable *hashtable;

	/* Allocate a new hash table structure */
	
	hashtable = (IrmoHashTable *) malloc(sizeof(IrmoHashTable));

	if (hashtable == NULL) {
		return NULL;
	}
	
	hashtable->hash_func = hash_func;
	hashtable->equal_func = equal_func;
	hashtable->key_free_func = NULL;
	hashtable->value_free_func = NULL;
	hashtable->entries = 0;
	hashtable->prime_index = 0;

	/* Allocate the table */

	if (!irmo_hash_table_allocate_table(hashtable)) {
		free(hashtable);

		return NULL;
	}

	return hashtable;
}

void irmo_hash_table_free(IrmoHashTable *hashtable)
{
	IrmoHashTableEntry *rover;
	IrmoHashTableEntry *next;
	int i;
	
	/* Free all entries in all chains */

	for (i=0; i<hashtable->table_size; ++i) {
		rover = hashtable->table[i];
		while (rover != NULL) {
			next = rover->next;
			irmo_hash_table_free_entry(hashtable, rover);
			rover = next;
		}
	}
	
	/* Free the table */

	free(hashtable->table);
	
	/* Free the hash table structure */

	free(hashtable);
}

void irmo_hash_table_register_free_functions(IrmoHashTable *hashtable,
                                        IrmoHashTableKeyFreeFunc key_free_func,
                                        IrmoHashTableValueFreeFunc value_free_func)
{
	hashtable->key_free_func = key_free_func;
	hashtable->value_free_func = value_free_func;
}


static int irmo_hash_table_enlarge(IrmoHashTable *hashtable)
{
	IrmoHashTableEntry **old_table;
	int old_table_size;
	int old_prime_index;
	IrmoHashTableEntry *rover;
	IrmoHashTableEntry *next;
	int index;
	int i;
	
	/* Store a copy of the old table */
	
	old_table = hashtable->table;
	old_table_size = hashtable->table_size;
	old_prime_index = hashtable->prime_index;

	/* Allocate a new, larger table */

	++hashtable->prime_index;
	
	if (!irmo_hash_table_allocate_table(hashtable)) {

		/* Failed to allocate the new table */

		hashtable->table = old_table;
		hashtable->table_size = old_table_size;
		hashtable->prime_index = old_prime_index;

		return 0;
	}

	/* Link all entries from all chains into the new table */

	for (i=0; i<old_table_size; ++i) {
		rover = old_table[i];

		while (rover != NULL) {
			next = rover->next;

			/* Find the index into the new table */
			
			index = hashtable->hash_func(rover->key) % hashtable->table_size;
			
			/* Link this entry into the chain */

			rover->next = hashtable->table[index];
			hashtable->table[index] = rover;
			
			/* Advance to next in the chain */

			rover = next;
		}
	}
       
	return 1;
}

int irmo_hash_table_insert(IrmoHashTable *hashtable, IrmoHashTableKey key, IrmoHashTableValue value) 
{
	IrmoHashTableEntry *rover;
	IrmoHashTableEntry *newentry;
	int index;
	
	/* If there are too many items in the table with respect to the table
	 * size, the number of hash collisions increases and performance
	 * decreases. Enlarge the table size to prevent this happening */

	if ((hashtable->entries * 3) / hashtable->table_size > 0) {
		
		/* Table is more than 1/3 full */

		if (!irmo_hash_table_enlarge(hashtable)) {

			/* Failed to enlarge the table */

			return 0;
		}
	}

	/* Generate the hash of the key and hence the index into the table */

	index = hashtable->hash_func(key) % hashtable->table_size;

	/* Traverse the chain at this location and look for an existing
	 * entry with the same key */

	rover = hashtable->table[index];

	while (rover != NULL) {
		if (hashtable->equal_func(rover->key, key) != 0) {

			/* Same key: overwrite this entry with new data */

			/* If there is a value free function, free the old data
			 * before adding in the new data */

			if (hashtable->value_free_func != NULL) {
				hashtable->value_free_func(rover->value);
			}

			/* Same with the key: use the new key value and free 
			 * the old one */

			if (hashtable->key_free_func != NULL) {
				hashtable->key_free_func(rover->key);
			}

			rover->key = key;
			rover->value = value;

			/* Finished */
			
			return 1;
		}
		rover = rover->next;
	}
	
	/* Not in the hashtable yet.  Create a new entry */

	newentry = (IrmoHashTableEntry *) malloc(sizeof(IrmoHashTableEntry));

	if (newentry == NULL) {
		return 0;
	}

	newentry->key = key;
	newentry->value = value;

	/* Link into the list */

	newentry->next = hashtable->table[index];
	hashtable->table[index] = newentry;

	/* Maintain the count of the number of entries */

	++hashtable->entries;

	/* Added successfully */

	return 1;
}

IrmoHashTableValue irmo_hash_table_lookup(IrmoHashTable *hashtable, IrmoHashTableKey key)
{
	IrmoHashTableEntry *rover;
	int index;

	/* Generate the hash of the key and hence the index into the table */
	
	index = hashtable->hash_func(key) % hashtable->table_size;

	/* Walk the chain at this index until the corresponding entry is
	 * found */

	rover = hashtable->table[index];

	while (rover != NULL) {
		if (hashtable->equal_func(key, rover->key) != 0) {

			/* Found the entry.  Return the data. */

			return rover->value;
		}
		rover = rover->next;
	}

	/* Not found */

	return HASH_TABLE_NULL;
}

int irmo_hash_table_remove(IrmoHashTable *hashtable, IrmoHashTableKey key)
{
	IrmoHashTableEntry **rover;
	IrmoHashTableEntry *entry;
	int index;
	int result;

	/* Generate the hash of the key and hence the index into the table */
	
	index = hashtable->hash_func(key) % hashtable->table_size;

	/* Rover points at the pointer which points at the current entry
	 * in the chain being inspected.  ie. the entry in the table, or
	 * the "next" pointer of the previous entry in the chain.  This
	 * allows us to unlink the entry when we find it. */

	result = 0;
	rover = &hashtable->table[index];

	while (*rover != NULL) {

		if (hashtable->equal_func(key, (*rover)->key) != 0) {

			/* This is the entry to remove */

			entry = *rover;

			/* Unlink from the list */

			*rover = entry->next;

			/* Destroy the entry structure */

			irmo_hash_table_free_entry(hashtable, entry);

			/* Track count of entries */

			--hashtable->entries;

			result = 1;

			break;
		}
		
		/* Advance to the next entry */

		rover = &((*rover)->next);
	}

	return result;
}

int irmo_hash_table_num_entries(IrmoHashTable *hashtable)
{
	return hashtable->entries;
}

IrmoHashTableIterator *irmo_hash_table_iterate(IrmoHashTable *hashtable)
{
	IrmoHashTableIterator *iterator;
	int chain;
	
	iterator = (IrmoHashTableIterator *) malloc(sizeof(IrmoHashTableIterator));

	if (iterator == NULL) {
		return NULL;
	}
	
	iterator->hashtable = hashtable;
	iterator->current_entry = NULL;

	/* Default value of next if no entries are found. */
	
	iterator->next_entry = NULL;
	
	/* Find the first entry */
	
	for (chain=0; chain<hashtable->table_size; ++chain) {
		
		if (hashtable->table[chain] != NULL) {
			iterator->next_entry = hashtable->table[chain];
			iterator->next_chain = chain;
			break;
		}
	}

	return iterator;
}

int irmo_hash_table_iter_has_more(IrmoHashTableIterator *iterator)
{
	return iterator->next_entry != NULL;
}

IrmoHashTableValue irmo_hash_table_iter_next(IrmoHashTableIterator *iterator)
{
	IrmoHashTable *hashtable;
	IrmoHashTableValue result;
	int chain;

	hashtable = iterator->hashtable;

	/* No more entries? */
	
	if (iterator->next_entry == NULL) {
		return HASH_TABLE_NULL;
	}
	
	/* Result is immediately available */

	iterator->current_entry = iterator->next_entry;
	result = iterator->current_entry->value;

	/* Find the next entry */

	if (iterator->current_entry->next != NULL) {
		
		/* Next entry in current chain */

		iterator->next_entry = iterator->current_entry->next;
		
	} else {
	
		/* None left in this chain, so advance to the next chain */

		chain = iterator->next_chain + 1;

		/* Default value if no next chain found */
		
		iterator->next_entry = NULL;

		while (chain < hashtable->table_size) {

			/* Is there anything in this chain? */

			if (hashtable->table[chain] != NULL) {
				iterator->next_entry = hashtable->table[chain];
				break;
			}

			/* Try the next chain */

			++chain;
		}

		iterator->next_chain = chain;
	}

	return result;
}

void irmo_hash_table_iter_free(IrmoHashTableIterator *iterator)
{
	free(iterator);
}

