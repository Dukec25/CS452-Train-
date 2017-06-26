#include <debug.h>
#include <log.h>
#include <string.h>

int dump_putc(char c)
{
	vint *index_addr = (vint *) DUMP_INDEX;
	vchar *dump_address = (vchar *) (DUMP_ADDRESS + *index_addr);
	*dump_address = c;
	*index_addr += 1;
	return 0;
}

void dump_putw(int n, char fc, char *bf)
{
	char ch;
	char *p = bf;

	while (*p++ && n > 0) n--;
	while (n-- > 0) dump_putc(fc);
	while ((ch = *bf++)) dump_putc(ch);
}

void dump_format(char *fmt, va_list va)
{
	char bf[12];
	char ch, lz;
	int w;

	
	while ((ch = *(fmt++))) {
		if (ch != '%')
			dump_putc(ch);
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
				dump_putc(va_arg(va, char));
				break;
			case 's':
				dump_putw(w, 0, va_arg(va, char*));
				break;
			case 'u':
				ui2a(va_arg(va, unsigned int), 10, bf);
				dump_putw(w, lz, bf);
				break;
			case 'd':
				i2a(va_arg(va, int), bf);
				dump_putw(w, lz, bf);
				break;
			case 'x':
				ui2a(va_arg(va, unsigned int), 16, bf);
				dump_putw(w, lz, bf);
				break;
			case '%':
				dump_putc(ch);
				break;
			}
		}
	}
}

void dump_printf(char *fmt, ...)
{
        va_list va;
        va_start(va,fmt);
        dump_format(fmt, va);
        va_end(va);
}
