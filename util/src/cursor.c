#include <define.h>
#include <io.h>
#include <cli.h>
#include <cursor.h>

#define ESC 0x1B

/* Busy wait */
void bw_cls()
{
	bwprintf(COM2, "%c[2J", ESC);
}
void bw_pos(int row, int col)
{
	bwprintf(COM2, "%c[%d;%dH", ESC, row, col);
}
void bw_save()
{
	bwprintf(COM2, "%c7", ESC);
}
void bw_restore()
{
	bwprintf(COM2, "%c8", ESC);
}

/* Non-busy wait */
void buffer_cls(fifo_t **pbuffer)
{
	buffer_printf(pbuffer, "%c[2J", ESC);
}
void buffer_pos(fifo_t **pbuffer, int row, int col)
{
	buffer_printf(pbuffer, "%c[%d;%dH", ESC, row, col);
}
void buffer_save(fifo_t **pbuffer)
{
	buffer_printf(pbuffer, "%c7", ESC);
}
void buffer_restore(fifo_t **pbuffer)
{
	buffer_printf(pbuffer, "%c8", ESC);
}
void buffer_nextline(fifo_t **pbuffer, int newlines)
{
	int offset = 0;
	offset = HEIGHT;
	// expand cursor to the next line
	buffer_pos(pbuffer, offset + newlines, 0);
	// move to the beginning of the next line
	buffer_putc(pbuffer, '>');
}
