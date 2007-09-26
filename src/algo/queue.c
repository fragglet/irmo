
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

#include "queue.h"

/* A double-ended queue */

typedef struct _IrmoQueueEntry IrmoQueueEntry;

struct _IrmoQueueEntry {
	void *data;
	IrmoQueueEntry *prev;
	IrmoQueueEntry *next;
};

struct _IrmoQueue {
	IrmoQueueEntry *head;
	IrmoQueueEntry *tail;
};

IrmoQueue *irmo_queue_new(void)
{
	IrmoQueue *queue;

	queue = (IrmoQueue *) malloc(sizeof(IrmoQueue));
	
	queue->head = NULL;
	queue->tail = NULL;

	return queue;
}

void irmo_queue_free(IrmoQueue *queue)
{
	/* Empty the queue */
	
	while (!irmo_queue_is_empty(queue)) {
		irmo_queue_pop_head(queue);
	}

	/* Free back the queue */

	free(queue);
}

void irmo_queue_push_head(IrmoQueue *queue, void *data)
{
	IrmoQueueEntry *new_entry;

	/* Create the new entry and fill in the fields in the structure */

	new_entry = malloc(sizeof(IrmoQueueEntry));
	new_entry->data = data;
	new_entry->prev = NULL;
	new_entry->next = queue->head;
	
	/* Insert into the queue */

	if (queue->head == NULL) {

		/* If the queue was previously empty, both the head and tail must
		 * be pointed at the new entry */

		queue->head = new_entry;
		queue->tail = new_entry;

	} else {

		/* First entry in the list must have prev pointed back to this
		 * new entry */

		queue->head->prev = new_entry;

		/* Only the head must be pointed at the new entry */

		queue->head = new_entry;
	}
}

void *irmo_queue_pop_head(IrmoQueue *queue)
{
	IrmoQueueEntry *entry;
	void *result;

	/* Check the queue is not empty */

	if (irmo_queue_is_empty(queue)) {
		return NULL;
	}

	/* Unlink the first entry from the head of the queue */

	entry = queue->head;
	queue->head = entry->next;
	result = entry->data;

	if (queue->head == NULL) {

		/* If doing this has unlinked the last entry in the queue, set
		 * tail to NULL as well. */

		queue->tail = NULL;
	} else {

		/* The new first in the queue has no previous entry */

		queue->head->prev = NULL;
	}

	/* Free back the queue entry structure */

	free(entry);
	
	return result;    
}

void *irmo_queue_peek_head(IrmoQueue *queue)
{
	if (irmo_queue_is_empty(queue)) {
		return NULL;
	} else {
		return queue->head->data;
	}
}

void irmo_queue_push_tail(IrmoQueue *queue, void *data)
{
	IrmoQueueEntry *new_entry;

	/* Create the new entry and fill in the fields in the structure */

	new_entry = malloc(sizeof(IrmoQueueEntry));
	new_entry->data = data;
	new_entry->prev = queue->tail;
	new_entry->next = NULL;
	
	/* Insert into the queue tail */

	if (queue->tail == NULL) {

		/* If the queue was previously empty, both the head and tail must
		 * be pointed at the new entry */

		queue->head = new_entry;
		queue->tail = new_entry;

	} else {

		/* The current entry at the tail must have next pointed to this
		 * new entry */

		queue->tail->next = new_entry;

		/* Only the tail must be pointed at the new entry */

		queue->tail = new_entry;
	}
}

void *irmo_queue_pop_tail(IrmoQueue *queue)
{
	IrmoQueueEntry *entry;
	void *result;

	/* Check the queue is not empty */

	if (irmo_queue_is_empty(queue)) {
		return NULL;
	}

	/* Unlink the first entry from the tail of the queue */

	entry = queue->tail;
	queue->tail = entry->prev;
	result = entry->data;

	if (queue->tail == NULL) {

		/* If doing this has unlinked the last entry in the queue, set
		 * head to NULL as well. */

		queue->head = NULL;

	} else {

		/* The new entry at the tail has no next entry. */

		queue->tail->next = NULL;
	}

	/* Free back the queue entry structure */

	free(entry);
	
	return result;    
}

void *irmo_queue_peek_tail(IrmoQueue *queue)
{
	if (irmo_queue_is_empty(queue)) {
		return NULL;
	} else {
		return queue->tail->data;
	}
}

int irmo_queue_is_empty(IrmoQueue *queue)
{
	return queue->head == NULL;
}

