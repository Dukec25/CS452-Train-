#include "int_fifo.h"

void int_fifo_init(int_fifo_t *pf)
{
    pf->head = 0;
    pf->tail = 0;
}

int is_int_fifo_empty(int_fifo_t *pf)
{
    return (pf->head == pf->tail);
}

int is_int_fifo_full(int_fifo_t *pf)
{
    int next = pf->head + 1;
    return (next == pf->tail); 
}

int int_fifo_put(int_fifo_t *pf, vint item)
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

int int_fifo_get(int_fifo_t *pf, vint *pitem)
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

int int_fifo_peek(int_fifo_t *pf, vint *pitem)
{
    if (is_fifo_empty(pf)) {
        return -1; // fifo Empty - nothing to get
    }
    *pitem = pf->items[pf->tail];
    return 0; // No errors
}

int int_fifo_get_count(int_fifo_t *pf)
{
    return pf->count;
}


