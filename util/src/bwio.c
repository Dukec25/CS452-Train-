/*
 * bwio.c - busy-wait I/O routines for diagnosis
 *
 * Specific to the TS-7200 ARM evaluation board
 *
 */

#include <ts7200.h>
#include <define.h>
#include <bwio.h>
#include <string.h>

void channel_select (int channel, vint **pflags, vint **pdata)
{
	switch(channel) {
	case COM1:
		*pflags = (vint *)(UART1_BASE + UART_FLAG_OFFSET);
		*pdata = (vint *)(UART1_BASE + UART_DATA_OFFSET);
		break;
	case COM2:
		*pflags = (vint *)(UART2_BASE + UART_FLAG_OFFSET);
		*pdata = (vint *)(UART2_BASE + UART_DATA_OFFSET);
		break;
	default:
		break;
	}
}

/*
 * The UARTs are initialized by RedBoot to the following state
 * 	115,200 bps
 * 	8 bits
 * 	no parity
 * 	fifos enabled
 */
int bwsetfifo(int channel, int state)
{
	vint *line, buf;
	switch (channel) {
	case COM1:
		line = (vint *)(UART1_BASE + UART_LCRH_OFFSET);
	        break;
	case COM2:
	        line = (vint *)(UART2_BASE + UART_LCRH_OFFSET);
	        break;
	default:
	        return -1;
	        break;
	}
	buf = *line;
	buf = state ? buf | FEN_MASK : buf & ~FEN_MASK;
	if (channel == COM1) {
		buf = (((buf | STP2_MASK) | WLEN_MASK) & (~PEN_MASK)) & (~BRK_MASK);
	}
	*line = buf;
	return 0;
}

int bwsetspeed(int channel, int speed)
{
	vint *high, *low;
	switch (channel) {
	case COM1:
		high = (vint *)(UART1_BASE + UART_LCRM_OFFSET);
		low = (vint *)(UART1_BASE + UART_LCRL_OFFSET);
	        break;
	case COM2:
		high = (vint *)(UART2_BASE + UART_LCRM_OFFSET);
		low = (vint *)(UART2_BASE + UART_LCRL_OFFSET);
	        break;
	default:
	        return -1;
	        break;
	}
	switch (speed) {
	case 115200:
		*high = 0x0;
		*low = 0x3;
		return 0;
	case 2400:
		*high = 0x0;
		*low = 0xBF;
		return 0;
	default:
		return -1;
	}
}

int bwputc(int channel, char c)
{
	vint *flags, *data;
	channel_select (channel, &flags, &data);
	if (channel == COM2) {
		while (*flags & TXFF_MASK) ;
	}
	else {
		while ((*flags & TXFF_MASK) || !(*flags & CTS_MASK)) ;
	}
	*data = c;
	return 0;
}

int bwputx(int channel, char c)
{
	char chh, chl;
	chh = c2x(c / 16);
	chl = c2x(c % 16);
	bwputc(channel, chh);
	return bwputc(channel, chl);
}

int bwputr(int channel, unsigned int reg)
{
	int byte;
	char *ch = (char *) &reg;

	for (byte = 3; byte >= 0; byte--) bwputx(channel, ch[byte]);
	return bwputc(channel, ' ');
}

int bwputstr(int channel, char *str)
{
	while (*str) {
		if (bwputc(channel, *str) < 0) return -1;
		str++;
	}
	return 0;
}

void bwputw(int channel, int n, char fc, char *bf)
{
	char ch;
	char *p = bf;

	while (*p++ && n > 0) n--;
	while (n-- > 0) bwputc(channel, fc);
	while ((ch = *bf++)) bwputc(channel, ch);
}

int bwgetc(int channel)
{
	unsigned char c;
	vint *flags, *data;
	channel_select (channel, &flags, &data);

	while (!(*flags & RXFF_MASK)) ;
	c = *data;
	return c;
}

void bwformat(int channel, char *fmt, va_list va)
{
	char bf[12];
	char ch, lz;
	int w;

	
	while ((ch = *(fmt++))) {
		if (ch != '%')
			bwputc(channel, ch);
		else {
			lz = 0; w = 0;
			ch = *(fmt++);
			switch (ch) {
			case '0':
				lz = 1; ch = *(fmt++);
				break;
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
				ch = a2i(ch, &fmt, 10, &w);
				break;
			}
			switch(ch) {
			case 0: return;
			case 'c':
				bwputc(channel, va_arg(va, char));
				break;
			case 's':
				bwputw(channel, w, 0, va_arg(va, char*));
				break;
			case 'u':
				ui2a(va_arg(va, unsigned int), 10, bf);
				bwputw(channel, w, lz, bf);
				break;
			case 'd':
				i2a(va_arg(va, int), bf);
				bwputw(channel, w, lz, bf);
				break;
			case 'x':
				ui2a(va_arg(va, unsigned int), 16, bf);
				bwputw(channel, w, lz, bf);
				break;
			case '%':
				bwputc(channel, ch);
				break;
			}
		}
	}
}

void bwprintf(int channel, char *fmt, ...)
{
        va_list va;

        va_start(va,fmt);
        bwformat(channel, fmt, va);
        va_end(va);
}
