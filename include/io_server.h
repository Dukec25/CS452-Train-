#ifndef __IO_SERVER_H__
#define __IO_SERVER_H__

#include <fifo.h>
#include <kernel.h>
#include <define.h>
#include <name_server.h>
#include <ts7200.h>
#include <clock_server.h>
#include <uart_irq.h>

typedef enum Io_message_type {
    RECEIVE_RDY,
    TRANSMIT_RDY,
    GETC,
    PUTC
} Io_message_type;

typedef struct Io_server{
    fifo_t get_q; // waiting tasks that want to getc
    fifo_t transmit_q; // characters waited to be transmit
    fifo_t receive_q; // input characters waited be return to tasks call getc
} Io_server;

#endif // __IO_SERVER_H__
