#ifndef __CURSOR_H__
#define __CURSOR_H__
#include <fifo.h>

/* Busy wait */
void bw_cls();
void bw_pos(int row, int col);
void bw_save();
void bw_restore();

/* IRQ */
void irq_cls();
void irq_pos(int row, int col);
void irq_save();
void irq_restore();
void irq_nextline(int newlines);

#endif // __CURSOR_H__
