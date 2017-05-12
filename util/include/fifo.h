#ifndef __FIFO_H__
#define __FIFO_H__

#define FIFO_SIZE 1000

typedef struct fifo_t {
	char bytes[FIFO_SIZE];
	int head;
	int tail;
} fifo_t;

fifo_t fifo_init();
int is_fifo_empty(fifo_t **pf);
int is_fifo_full(fifo_t **pf);
int fifo_put(fifo_t **pf, char c);
int fifo_get(fifo_t **pf, char *c);
int fifo_peek(fifo_t **pf, char *c);
#endif // __FIFO_H__
