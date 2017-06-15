#ifndef __LIFO_H__
#define __LIFO_H__

#define LIFO_SIZE 1000

typedef struct Lifo_t {
    void *items[LIFO_SIZE];
    int top;
} Lifo_t;

void lifo_init(Lifo_t *target);
void lifo_push(Lifo_t *target, void *item);
int lifo_pop(Lifo_t *target, void **pitem);
int is_lifo_empty(Lifo_t *target);

#endif // __LIFO_H__


