#ifndef __CURSOR_H__
#define __CURSOR_H__
#include <fifo.h>

/* Busy wait */
void bw_cls();
void bw_pos(int row, int col);
void bw_save();
void bw_restore();

/* Non-busy wait */
void buffer_cls(fifo_t **pbuffer);
void buffer_pos(fifo_t **pbuffer, int row, int col);
void buffer_save(fifo_t **pbuffer);
void buffer_restore(fifo_t **pbuffer);
void buffer_nextline(fifo_t **pbuffer, int newlines);

#endif // __CURSOR_H__
