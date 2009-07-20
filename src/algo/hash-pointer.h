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
 * @file hash-pointer.h
 *
 * Hash function for a generic (void) pointer.  See @ref irmo_pointer_hash.
 */

#ifndef IRMO_ALGO_HASH_POINTER_H
#define IRMO_ALGO_HASH_POINTER_H

#ifdef __cplusplus
extern "C" {
#endif

/** 
 * Generate a hash key for a pointer.  The value pointed at by the pointer
 * is not used, only the pointer itself.
 *
 * @param location        The pointer
 * @return                A hash key for the pointer.
 */

unsigned long irmo_pointer_hash(void *location);

/** Macro to cast integer values for use as pointers. */

#define IRMO_POINTER_KEY(val) ((void *) ((long) (val)))

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ALGO_HASH_POINTER_H */

