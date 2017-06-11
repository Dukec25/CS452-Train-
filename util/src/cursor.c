#include <define.h>
#include <bwio.h>
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

/* IRQ */
void irq_cls()
{
	irq_printf(COM2, "%c[2J", ESC);
}
void irq_pos(int row, int col)
{
	irq_printf(COM2, "%c[%d;%dH", ESC, row, col);
}
void irq_save()
{
	irq_printf(COM2, "%c7", ESC);
}
void irq_restore()
{
	irq_printf(COM2, "%c8", ESC);
}
void irq_nextline(int newlines)
{
	int offset = 0;
	offset = HEIGHT;
	// expand cursor to the next line
	irq_pos(offset + newlines, 0);
	// move to the beginning of the next line
	irq_putc(COM2, '>');
}
