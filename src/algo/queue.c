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

#include "queue.h"

/* malloc() / free() testing */

#ifdef ALLOC_TESTING
#include "alloc-testing.h"
#endif

/* A double-ended queue */

typedef struct _IrmoQueueEntry IrmoQueueEntry;

struct _IrmoQueueEntry {
	IrmoQueueValue data;
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

	if (queue == NULL) {
		return NULL;
	}
	
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

int irmo_queue_push_head(IrmoQueue *queue, IrmoQueueValue data)
{
	IrmoQueueEntry *new_entry;

	/* Create the new entry and fill in the fields in the structure */

	new_entry = malloc(sizeof(IrmoQueueEntry));

	if (new_entry == NULL) {
		return 0;
	}
	
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

	return 1;
}

IrmoQueueValue irmo_queue_pop_head(IrmoQueue *queue)
{
	IrmoQueueEntry *entry;
	IrmoQueueValue result;

	/* Check the queue is not empty */

	if (irmo_queue_is_empty(queue)) {
		return IRMO_QUEUE_NULL;
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

IrmoQueueValue irmo_queue_peek_head(IrmoQueue *queue)
{
	if (irmo_queue_is_empty(queue)) {
		return IRMO_QUEUE_NULL;
	} else {
		return queue->head->data;
	}
}

int irmo_queue_push_tail(IrmoQueue *queue, IrmoQueueValue data)
{
	IrmoQueueEntry *new_entry;

	/* Create the new entry and fill in the fields in the structure */

	new_entry = malloc(sizeof(IrmoQueueEntry));

	if (new_entry == NULL) {
		return 0;
	}
	
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

	return 1;
}

IrmoQueueValue irmo_queue_pop_tail(IrmoQueue *queue)
{
	IrmoQueueEntry *entry;
	IrmoQueueValue result;

	/* Check the queue is not empty */

	if (irmo_queue_is_empty(queue)) {
		return IRMO_QUEUE_NULL;
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

IrmoQueueValue irmo_queue_peek_tail(IrmoQueue *queue)
{
	if (irmo_queue_is_empty(queue)) {
		return IRMO_QUEUE_NULL;
	} else {
		return queue->tail->data;
	}
}

int irmo_queue_is_empty(IrmoQueue *queue)
{
	return queue->head == NULL;
}

