#ifndef __IRQ_IO_H__
#define __IRQ_IO_H__
#include <define.h>
#include <bwio.h>

/* Interrupt I/O */
void irq_putw(int channel, int n, char fc, char *bf);
void irq_printf(int channel, char *fmt, ...);
void irq_format(int channel, char *fmt, va_list va);
#endif // __BWIO_H__
