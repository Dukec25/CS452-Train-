#include <uart_irq.h>

static uint32 uart1_irq_mask()
{
	return 0x1 << (UART1_GENERAL_INTERRUPT - 33);
}

void uart1_irq_soft()
{
	vint *vic2_soft_int = (vint *) VIC2_SOFT_INT;
	*vic2_soft_int |= uart1_irq_mask();
}

void uart1_irq_soft_clear()
{
	vint *vic2_soft_int_clr = (vint *) VIC2_SOFT_INT_CLR;
	*vic2_soft_int_clr |= 0xFFFFFFFF;
}

void uart1_irq_enable()
{
    mode_irq();
    uart1_vic_enable();
//    uart1_device_enable();
}

void uart1_irq_disable()
{
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl &= ~uart1_irq_mask();
	vint *vic_int_enbl_clr = (vint *) VIC2_INT_ENBL_CLR;
	*vic_int_enbl_clr |= uart1_irq_mask();
}

void mode_irq(){
	debug(DEBUG_IRQ, "enter %s", "irq_enable");
	vint *vic2_int_sel = (vint *) VIC2_INT_SEL;
	*vic2_int_sel &= 0x0;	// interrupt type = IRQ
}

void uart1_vic_enable()
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_irq_enable");
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl |= uart1_irq_mask();
	debug(DEBUG_UART_IRQ, "*vic2_int_enbl = 0x%x, uart1_irq_mask = 0x%x", *vic2_int_enbl, uart1_irq_mask());
}

void uart1_device_enable()
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_device_enable");
	vint *uart1_ctrl = (vint *) UART1_CTRL;
    // receive interrupt 
    /**uart1_ctrl |= RIEN_MASK;*/ 
    // transmit interrupt
    *uart1_ctrl |= TIEN_MASK;
	*uart1_ctrl |= UARTEN_MASK;
}

void uart1_device_disable()
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_device_disable");
	vint *uart1_ctrl = (vint *) UART1_CTRL;
    // transmit interrupt
    *uart1_ctrl &= ~TIEN_MASK;
	*uart1_ctrl &= ~UARTEN_MASK;
}

int Getc(int channel){
	/*vint *flags, *data;*/
    /*channel_select(channel, &flags, &data);*/
	int io_server_channel2_id = WhoIs("IO_SERVER_CHANNEL2");
    debug(DEBUG_UART_IRQ, "enter Getc, server is %d, type = %d", io_server_channel2_id, GETC);
    Delivery request;
    request.type = GETC;
    Delivery reply_msg;
    Send(io_server_channel2_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg) );
    return reply_msg.data;
}

int Putc(int channel, char ch){
	int io_server_channel2_id = WhoIs("IO_SERVER_CHANNEL2");
    Delivery request;
    request.type = PUTC;
    request.data = ch;
    Delivery reply_msg;
	debug(DEBUG_UART_IRQ, "send %d to io_server_channel2_id %d", ch, io_server_channel2_id);
    Send(io_server_channel2_id, &request, sizeof(request), &reply_msg, sizeof(reply_msg));
	debug(DEBUG_UART_IRQ, "received reply_msg.data = %d", reply_msg.data);
    return 1;
}
