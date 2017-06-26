#ifndef __BWIO_H__
#define __BWIO_H__
#include <define.h>
/* Busy-wait I/O */
int bwsetfifo(int channel, int state);
int bwsetspeed(int channel, int speed);
int bwputc(int channel, char c);
int bwgetc(int channel);
int bwputx(int channel, char c);
int bwputstr(int channel, char *str);
int bwputr(int channel, unsigned int reg);
void bwputw(int channel, int n, char fc, char *bf);
void bwformat (int channel, char *fmt, va_list va);
void bwprintf(int channel, char *format, ...);
void channel_select(int channel, vint **ppflags, vint **ppdata);

#endif // __BWIO_H__
