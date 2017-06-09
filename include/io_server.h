#ifndef __IO_SERVER_H__
#define __IO_SERVER_H__

#include <fifo.h>
#include <kernel.h>
#include <define.h>
#include <name_server.h>

typedef enum Io_message_type {
    CLOCK_NOTIFIER,
    TIME_REQUEST,
    DELAY_REQUEST,
    DELAY_REQUEST_UNTIL
} Io_message_type;

typedef struct Io_server{
    fifo_t get_q;
    fifo_t transmit_q;
    fifo_t receive_q;
}

#endif // __IO_SERVER_H_
