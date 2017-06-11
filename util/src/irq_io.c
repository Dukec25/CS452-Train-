#include <irq_io.h>
#include <user_functions.h>

void irq_format(int channel, char *fmt, va_list va){
	char bf[12];
	char ch, lz;
	int w;
	
	while ((ch = *(fmt++))) {
		if (ch != '%')
			Putc(channel, ch);
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
				Putc(channel, va_arg(va, char));
				break;
			case 's':
				irq_putw(channel, w, 0, va_arg(va, char*));
				break;
			case 'u':
				ui2a(va_arg(va, unsigned int), 10, bf);
				irq_putw(channel, w, lz, bf);
				break;
			case 'd':
				i2a(va_arg(va, int), bf);
				irq_putw(channel, w, lz, bf);
				break;
			case 'x':
				ui2a(va_arg(va, unsigned int), 16, bf);
				irq_putw(channel, w, lz, bf);
				break;
			case '%':
				Putc(channel, ch);
				break;
			}
		}
	}
}

void irq_putw(int channel, int n, char fc, char *bf)
{
	char ch;
	char *p = bf;

	while (*p++ && n > 0) n--;
	while (n-- > 0) Putc(channel, fc);
	while ((ch = *bf++)) Putc(channel, ch);
}

void irq_printf(int channel, char *fmt, ...){
    va_list va;

    va_start(va,fmt);
    irq_format(channel, fmt, va);
    va_end(va);
}
