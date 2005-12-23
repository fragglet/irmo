
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
	void *key;
	void *value;
	IrmoHashTableEntry *next;
};

struct _IrmoHashTable {
	IrmoHashTableEntry **table;
	int table_size;
	IrmoHashTableHashFunc hash_func;
	IrmoHashTableEqualFunc equal_func;
	IrmoHashTableFreeFunc key_free_func;
	IrmoHashTableFreeFunc value_free_func;
	int entries;
	int prime_index;
};

/* Prime numbers on an escalating exponential scale, used for the table
 * size.  Each value is approximately 1.5 * the previous value, so the
 * table size increases by 50% with each enlargement */

static unsigned int irmo_hash_table_primes[] = {
	251, 383, 571, 863, 1291, 1933, 2909, 4373, 6553, 9839, 14759, 22133,
	33211, 49811, 74719, 112069, 168127, 252193, 378289, 567407, 851131,
	1276721, 1915057, 2872621, 4308937, 6463399, 9695099, 14542651,
	21813997, 32721001, 49081441, 73622251, 110433383, 165650033,
	248475107, 372712667, 559068997, 838603499, 1257905249, 1886857859,
};

static int irmo_hash_table_num_primes = sizeof(irmo_hash_table_primes) / sizeof(int);

/* Internal function used to allocate the table on hashtable creation
 * and when enlarging the table */

static void irmo_hash_table_allocate_table(IrmoHashTable *hashtable)
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
	memset(hashtable->table, 0, 
	       hashtable->table_size * sizeof(IrmoHashTableEntry *));
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
	hashtable->hash_func = hash_func;
	hashtable->equal_func = equal_func;
	hashtable->key_free_func = NULL;
	hashtable->value_free_func = NULL;
	hashtable->entries = 0;
	hashtable->prime_index = 0;

	/* Allocate the table */

	irmo_hash_table_allocate_table(hashtable);

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
                                        IrmoHashTableFreeFunc key_free_func,
                                        IrmoHashTableFreeFunc value_free_func)
{
	hashtable->key_free_func = key_free_func;
	hashtable->value_free_func = value_free_func;
}


static void irmo_hash_table_enlarge(IrmoHashTable *hashtable)
{
	IrmoHashTableEntry **old_table;
	int old_table_size;
	IrmoHashTableEntry *rover;
	IrmoHashTableEntry *next;
	int index;
	int i;
	
	/* Store a copy of the old table */
	
	old_table = hashtable->table;
	old_table_size = hashtable->table_size;

	/* Allocate a new, larger table */

	++hashtable->prime_index;
	irmo_hash_table_allocate_table(hashtable);

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
}

void irmo_hash_table_insert(IrmoHashTable *hashtable, void *key, void *value) 
{
	IrmoHashTableEntry *rover;
	IrmoHashTableEntry *newentry;
	int index;
	
	/* If there are too many items in the table with respect to the table
	 * size, the number of hash collisions increases and performance
	 * decreases. Enlarge the table size to prevent this happening */

	if ((hashtable->entries * 3) / hashtable->table_size > 0) {
		
		/* Table is more than 1/3 full */

		irmo_hash_table_enlarge(hashtable);
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

			/* Same with the key: use the new key value and free the 
			 * old one */

			if (hashtable->key_free_func != NULL) {
				hashtable->key_free_func(rover->key);
			}

			rover->key = key;
			rover->value = value;

			/* Finished */
			
			return;
		}
		rover = rover->next;
	}
	
	/* Not in the hashtable yet.  Create a new entry */

	newentry = (IrmoHashTableEntry *) malloc(sizeof(IrmoHashTableEntry));

	newentry->key = key;
	newentry->value = value;

	/* Link into the list */

	newentry->next = hashtable->table[index];
	hashtable->table[index] = newentry;

	/* Maintain the count of the number of entries */

	++hashtable->entries;
}

void *irmo_hash_table_lookup(IrmoHashTable *hashtable, void *key)
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

	return NULL;
}

int irmo_hash_table_remove(IrmoHashTable *hashtable, void *key)
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

void irmo_hash_table_foreach(IrmoHashTable *hashtable, IrmoHashTableIterator iterator,
                        void *user_data)
{
	int i;
	IrmoHashTableEntry *rover;

	/* Iterate over all entries in all chains */

	for (i=0; i<hashtable->table_size; ++i) {
		rover = hashtable->table[i];

		while (rover != NULL) {
			iterator(rover->key, rover->value, user_data);
			rover = rover->next;
		}
	}
}

int irmo_hash_table_foreach_remove(IrmoHashTable *hashtable,
                              IrmoHashTableRemoveIterator iterator,
                              void *user_data)
{
	int i;
	int removed_entries;
	int remove;
	IrmoHashTableEntry **rover;
	IrmoHashTableEntry *entry;

	/* Iterate over all entries in all chains */

	removed_entries = 0;

	for (i=0; i<hashtable->table_size; ++i) {
		rover = &(hashtable->table[i]);

		while (*rover != NULL) {
			
			entry = *rover;

			remove = iterator(entry->key, entry->value, user_data);

			/* Remove this entry? */

			if (remove) {

				/* Unlink this entry from the chain */

				*rover = entry->next;
				--hashtable->entries;

				/* Destroy the entry structure */

				irmo_hash_table_free_entry(hashtable, entry);

				/* Keep count of the number removed */

				++removed_entries;
				
			} else {

				/* Advance to the next entry in the chain */

				rover = &((*rover)->next);
			}
		}
	}

	return removed_entries;
}


