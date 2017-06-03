#ifndef __HEAP_H__
#define __HEAP_H__
#include <define.h>
#include <debug.h>

/* heap priority queue that return the least priority */

/*
 * min heap node
 */
typedef struct {
	int priority;
	void *data;
} node_t;

/*
 * min heap
 */
typedef struct {
	node_t *nodes;
	uint32 len;
	uint32 size;
} heap_t; 

/*
 * Predicate to check whether min heap is full
 */
int is_heap_full(heap_t *ph);

/*
 * Predicate to check whether min heap is empty
 */
int is_heap_empty(heap_t *ph);

/*
 * Returns an min heap initialized nodes and size
 */
heap_t heap_init(node_t *nodes, int size);

/*
 * Returns the min node of the min heap
 */
node_t heap_root(heap_t *ph);

/*
 * Returns -1 if min heap is full.
 * Returns 0 otherwise, and inserts the node with priority and data into the min heap.
 */
int heap_insert(heap_t *ph, int priority, void *data);

/*
 * Returns -1 if min heap is empty.
 * Returns 0 otherwise, and deletes the min node and stores it into proot.
 */
int heap_delete(heap_t *ph, node_t *proot);

void heap_print(heap_t *ph);
#endif // __HEAP_H__
