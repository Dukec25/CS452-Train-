#include <lifo.h>

void lifo_init(Lifo_t *target){
    target->top = -1; 
}

void lifo_push(Lifo_t *target, void *item){
    if(target->top == LIFO_SIZE - 1){
        // stack is full 
    }
    target->top = target->top + 1;
    target->items[target->top] = item; 
}

int lifo_pop(Lifo_t *target, void **pitem){
    if(target->top == -1){
        return -1;
        // stack is empty 
    }
    *pitem = target->items[target->top];
    target->top = target->top - 1; 
    return 0; // no error
}

int is_lifo_empty(Lifo_t *target){
    if(target->top == -1){
        return 1;
    } 
    return 0;
}


