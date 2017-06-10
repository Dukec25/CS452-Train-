#include <uart_irq.h>

uint32 uart_irq_mask(int channel)
{
	switch (channel) {
	case COM1:
		return 0x1 << (UART1_GENERAL_INTERRUPT - 33);
	case COM2:
		return 0x1 << (UART2_GENERAL_INTERRUPT - 33);
	default:
		return 0;
	}
}

void uart_irq_soft(int channel)
{
	vint *vic2_soft_int = (vint *) VIC2_SOFT_INT;
	*vic2_soft_int |= uart_irq_mask(channel);
}

void uart_irq_soft_clear()
{
	vint *vic2_soft_int_clr = (vint *) VIC2_SOFT_INT_CLR;
	*vic2_soft_int_clr |= 0xFFFFFFFF;
}

void mode_irq(){
	debug(DEBUG_IRQ, "enter %s", "irq_enable");
	vint *vic2_int_sel = (vint *) VIC2_INT_SEL;
	*vic2_int_sel &= 0x0;	// interrupt type = IRQ
}

void uart_vic_enable(int channel)
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_irq_enable");
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl |= uart_irq_mask(channel);
	debug(DEBUG_UART_IRQ, "*vic2_int_enbl = 0x%x", *vic2_int_enbl);
}

void uart_irq_enable(int channel)
{
    mode_irq();
    uart_vic_enable(channel);
}

void uart_irq_disable(int channel)
{
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl &= ~uart_irq_mask(channel);
	vint *vic_int_enbl_clr = (vint *) VIC2_INT_ENBL_CLR;
	*vic_int_enbl_clr |= uart_irq_mask(channel);
}

void uart_device_enable(int channel, UART_IRQ_TYPE type)
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_device_enable");
	vint *uart_ctrl;
	uint32 mask;
	switch (channel) {
	case COM1:
		uart_ctrl = (vint *) UART1_CTRL;
		break;
	case COM2:
		uart_ctrl = (vint *) UART2_CTRL;
		break;
	}
	switch (type) {
	case XMIT:
		mask = TIEN_MASK;
	case RCV:
		mask = RIEN_MASK;
	}
    *uart_ctrl |= mask; 
	debug(DEBUG_UART_IRQ, "uart_ctrl = 0x%x, *uart_ctrl = 0x%x", uart_ctrl, *uart_ctrl);
}

void uart_device_disable(int channel, UART_IRQ_TYPE type)
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_device_disable");
	vint *uart_ctrl;
	uint32 mask;
	switch (channel) {
	case COM1:
		uart_ctrl = (vint *) UART1_CTRL;
		break;
	case COM2:
		uart_ctrl = (vint *) UART2_CTRL;
		break;
	}
	switch (type) {
	case XMIT:
		mask = TIEN_MASK;
	case RCV:
		mask = RIEN_MASK;
	}
    *uart_ctrl &= ~mask; 
}

int Getc(int channel){
	int io_server_id;
	switch (channel) {
	case COM1:
		io_server_id = WhoIs("IO_SERVER_UART1_RECEIVE");
		break;
	case COM2:
		io_server_id = WhoIs("IO_SERVER_UART2_RECEIVE");
		break;
	}
    debug(DEBUG_UART_IRQ, "enter Getc, server is %d, type = %d", io_server_id, GETC);
    Delivery request;
    request.type = GETC;
    Delivery reply_msg;
    Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg) );
    return reply_msg.data;
}

int Putc(int channel, char ch){
	int io_server_id;
	switch (channel) {
	case COM1:
		io_server_id = WhoIs("IO_SERVER_UART1_TRANSMIT");
		break;
	case COM2:
		io_server_id = WhoIs("IO_SERVER_UART2_TRANSMIT");
		break;
	}
    debug(DEBUG_UART_IRQ, "enter Putc, server is %d, type = %d", io_server_id, PUTC);
    Delivery request;
    request.type = PUTC;
    request.data = ch;
    Delivery reply_msg;
	debug(DEBUG_UART_IRQ, "send %d to io_server_id %d", ch, io_server_id);
    Send(io_server_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
	debug(DEBUG_UART_IRQ, "received reply_msg.data = %d", reply_msg.data);
    return 1;
}
