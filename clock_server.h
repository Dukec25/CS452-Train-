#ifndef __CLOCK_SERVER_H__
#define __CLOCK_SERVER_H__

#include <fifo.h>
#include <kernel.h>
#include <define.h>

typedef enum Clock_message_type {
    CLOCK_NOTIFIER,
    TIME_REQUEST,
    DELAY_REQUEST,
    DELAY_REQUEST_UNTIL,
	MSG_SUCCESS,
	MSG_FAILURE,
} Clock_message_type;

typedef struct Clock_server_message {
	Clock_message_type type;
    vint ticks; 
} Clock_server_message;

typedef struct Clock_server {
	vint ticks;
	fifo_t delayed_task_queue; // should here really be fifo, the heap structured priority queue maybe better
} Name_server;

typedef struct Delayed_task {
	vint tid;
    vint freedom_tick;
} Clock_server_message;

typedef struct Delivery{
    vint type;
    vint data;
} Delivery;

int DelayUntil( int ticks );
int Delay( int ticks );
int Time();

#endif // __CLOCK_SERVER_H__
