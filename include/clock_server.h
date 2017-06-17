#ifndef __CLOCK_SERVER_H__
#define __CLOCK_SERVER_H__
#include <define.h>
#include <user_functions.h>
#include <debug.h>
#include <heap.h>

#define MAX_DELAYED_TASKS	50

typedef enum Clock_message_type {
    CLOCK_NOTIFIER,
    TIME_REQUEST,
    DELAY_REQUEST,
    DELAY_REQUEST_UNTIL
} Clock_message_type;

typedef struct Clock_server_message {
	Clock_message_type type;
    vint ticks; 
} Clock_server_message;

typedef struct Clock_server {
	vint ticks;
	//node_t delayed_tasks[MAX_DELAYED_TASKS];
} Clock_server;

typedef struct Delayed_task {
	vint tid;
    vint freedom_tick;
} Delayed_task;

typedef struct Delivery{
    vint type;
    vint data;
    char *data_arr;
} Delivery;

#endif // __CLOCK_SERVER_H__
