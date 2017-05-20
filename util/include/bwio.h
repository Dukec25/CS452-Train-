#ifndef __BWIO_H__
#define __BWIO_H__
#include <define.h>
typedef char *va_list;

#define __va_argsiz(t)	\
		(((sizeof(t) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(ap, pN) ((ap) = ((va_list) __builtin_next_arg(pN)))

#define va_end(ap)	((void)0)

#define va_arg(ap, t)	\
		 (((ap) = (ap) + __va_argsiz(t)), *((t*) (void*) ((ap) - __va_argsiz(t))))

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

/* debug */
#define debug(fmt, ...) 													 			\
		do {																			\
			if (DEBUG) bwprintf(COM2, "\r\n%s:%d " fmt, __FILE__, __LINE__, __VA_ARGS__);	\
			}																			\
		while (0)

#endif // __BWIO_H__
