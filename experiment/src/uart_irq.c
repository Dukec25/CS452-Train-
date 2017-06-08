#include <uart_irq.h>
#include <debug.h>
#include <kernel.h>
#include <irq.h>

static uint32 uart2_irq_mask()
{
	return 0x1 << (UART2_GENERAL_INTERRUPT - 33);
}

void uart2_irq_soft()
{
	vint *vic2_soft_int = (vint *) VIC2_SOFT_INT;
	*vic2_soft_int |= uart2_irq_mask();
}

void uart2_irq_soft_clear()
{
	vint *vic2_soft_int_clr = (vint *) VIC2_SOFT_INT_CLR;
	*vic2_soft_int_clr |= 0xFFFFFFFF;
}

void uart2_irq_enable()
{
    mode_irq();
    uart2_vic_enable();
    uart2_device_enable();
}

void mode_irq(){
	debug(DEBUG_IRQ, "enter %s", "irq_enable");
	vint *vic2_int_sel = (vint *) VIC2_INT_SEL;
	*vic2_int_sel &= 0x0;	// interrupt type = IRQ
}

void uart2_vic_enable()
{
	/*debug(DEBUG_UART_IRQ, "enter %s", "uart_irq_enable");*/
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl |= uart2_irq_mask();
}

void uart2_device_enable()
{
	/*debug(DEBUG_UART_IRQ, "enter %s", "uart_device_enable");*/
	vint *uart2_ctrl = (vint *) UART2_CTRL;
    *uart2_ctrl |= RIEN_MASK;
    *uart2_ctrl |= TIEN_MASK;
}

int Getc(int server_tid, int channel){
	vint *flags, *data;
    channel_select(channel, &flags, &data);

}

int Putc(int server_tid, int channel, char ch){
	vint *flags, *data;
    channel_select(channel, &flags, &data);

}


