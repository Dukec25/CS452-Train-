#ifndef __HEAP_H__
#define __HEAP_H__
#include <define.h>

/*
 * Max heap node
 */
typedef struct {
	int priority;
	void *data;
} node_t;

/*
 * Max heap
 */
typedef struct {
	node_t *nodes;
	uint32 len;
	uint32 size;
} heap_t; 

/*
 * Predicate to check whether max heap is full
 */
int is_heap_full(heap_t *ph);

/*
 * Predicate to check whether max heap is empty
 */
int is_heap_empty(heap_t *ph);

/*
 * Returns an max heap initialized nodes and size
 */
heap_t heap_init(node_t *nodes, int size);

/*
 * Returns the max node of the max heap
 */
node_t heap_root(heap_t *ph);

/*
 * Returns -1 if max heap is full.
 * Returns 0 otherwise, and inserts the node with priority and data into the max heap.
 */
int heap_insert(heap_t *ph, int priority, void *data);

/*
 * Returns -1 if max heap is empty.
 * Returns 0 otherwise, and deletes the max node and stores it into proot.
 */
int heap_delete(heap_t *ph, node_t *proot);

#endif // __HEAP_H__
