#ifndef __FIFO_H__
#define __FIFO_H__

#define FIFO_SIZE 1000

typedef struct fifo_t {
	void *items[FIFO_SIZE];
	int head;
	int tail;
	int count;
} fifo_t;

void fifo_init(fifo_t *pf);
int is_fifo_empty(fifo_t *pf);
int is_fifo_full(fifo_t *pf);
int fifo_put(fifo_t *pf, void *item);
int fifo_get(fifo_t *pf, void **pitem);
int fifo_peek(fifo_t *pf, void **pitem);
int fifo_get_count(fifo_t *pf);
#endif // __FIFO_H__
