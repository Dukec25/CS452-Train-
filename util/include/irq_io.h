#ifndef __IRQ_IO_H__
#define __IRQ_IO_H__
#include <bwio.h>
#include <user_functions.h>
#include <clock_server.h>
#include <io_server.h>

/* Interrupt I/O */
void irq_printf(int channel, char *fmt, ...);
void irq_format(int channel, char *fmt, va_list va);
int irq_putw(int n, char fc, char *bf, char *val, int idx);
#endif // __BWIO_H__
