#include "fifo.h"

void fifo_init(fifo_t *pf)
{
	pf->head = 0;
	pf->tail = 0;
}

int is_fifo_empty(fifo_t *pf)
{
	return (pf->head == pf->tail);
}

int is_fifo_full(fifo_t *pf)
{
	int next = pf->head + 1;
	return (next == pf->tail); 
}

int fifo_put(fifo_t *pf, void *item)
{
	if (is_fifo_full(pf)) {
	    return -1; // fifo full
	}

	int next = pf->head + 1;
	if (next >= FIFO_SIZE) {
		next = 0;
	}
	pf->items[pf->head] = item;
	pf->head = next;
	pf->count++;
	return 0; // No errors
}

int fifo_get(fifo_t *pf, void **pitem)
{
	if (is_fifo_empty(pf)) {
	    return -1; // fifo Empty - nothing to get
	}

	int next = pf->tail + 1;
	if (next >= FIFO_SIZE) {
		next = 0;
	}
	*pitem = pf->items[pf->tail];
	pf->tail = next;
	pf->count--;
	return 0; // No errors
}

int fifo_peek(fifo_t *pf, void **pitem)
{
	if (is_fifo_empty(pf)) {
	    return -1; // fifo Empty - nothing to get
	}
	*pitem = pf->items[pf->tail];
	return 0; // No errors
}

int fifo_get_count(fifo_t *pf)
{
	return pf->count;
}
