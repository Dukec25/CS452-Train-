#ifndef __IO_SERVER_H__
#define __IO_SERVER_H__
#include <fifo.h>

typedef enum Io_message_type {
    RECEIVE_RDY,
    TRANSMIT_RDY,
    GETC,
    PUTC, 
    PRINTF
} Io_message_type;

typedef struct Io_server{
    fifo_t get_q; // waiting tasks that want to getc
    fifo_t transmit_q; // characters waited to be transmit
    fifo_t receive_q; // input characters waited be return to tasks call getc
} Io_server;


void io_server_receive_start(int channel);
void io_server_transmit_start(int channel);
#endif // __IO_SERVER_H__
