#include "fifo.h"

fifo_t fifo_init()
{
	fifo_t f;
	f.head = 0;
	f.tail = 0;
	return f;
}

int is_fifo_empty(fifo_t **pf)
{
	return ((*pf)->head == (*pf)->tail);
}

int is_fifo_full(fifo_t **pf)
{
	int next = (*pf)->head + 1;
	return (next == (*pf)->tail); 
}

int fifo_put(fifo_t **pf, char c)
{
	if (is_fifo_full(pf)) {
	    return -1; // fifo full
	}

	int next = (*pf)->head + 1;
	if (next >= FIFO_SIZE) {
		next = 0;
	}
	(*pf)->bytes[(*pf)->head] = c;
	(*pf)->head = next;
	return 0; // No errors
}

int fifo_get(fifo_t **pf, char *c)
{
	if (is_fifo_empty(pf)) {
	    return -1; // fifo Empty - nothing to get
	}

	int next = (*pf)->tail + 1;
	if (next >= FIFO_SIZE) {
		next = 0;
	}
	*c = (*pf)->bytes[(*pf)->tail];
	(*pf)->tail = next;
	return 0; // No errors
}

int fifo_peek(fifo_t **pf, char *c)
{
	if (is_fifo_empty(pf)) {
	    return -1; // fifo Empty - nothing to get
	}
	*c = (*pf)->bytes[(*pf)->tail];
	return 0; // No errors
}
