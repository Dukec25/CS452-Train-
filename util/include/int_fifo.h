#ifndef __INT_FIFO_H__
#define __INT_FIFO_H__
#define FIFO_SIZE 3000

#include <define.h>

typedef struct int_fifo_t {
    vint items[FIFO_SIZE];
    int head;
    int tail;
    int count;
} int_fifo_t;

void int_fifo_init(int_fifo_t *pf);
int is_int_fifo_empty(int_fifo_t *pf);
int is_int_fifo_full(int_fifo_t *pf);
int int_fifo_put(int_fifo_t *pf, vint item);
int int_fifo_get(int_fifo_t *pf, vint *pitem);
int int_fifo_peek(int_fifo_t *pf, vint *pitem);
int int_fifo_get_count(int_fifo_t *pf);

#endif // __INT_FIFO_H__


