#include <irq.h>
#include <uart_irq.h>

static uint32 uart_receive_irq_mask()
{
    return 0x1 << 1;
}

static uint32 uart_transmit_irq_mask()
{
    return 0x1 << 2;
}

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
	vint *vic2_int_enbl_clr = (vint *) VIC2_INT_ENBL_CLR;
	*vic2_int_enbl_clr |= uart_irq_mask(channel);
}

void uart_device_enable(int channel, UART_IRQ_TYPE type)
{
	debug(DEBUG_UART_IRQ, "enter uart_device_enable, channel = %d, type = %d", channel, type);
	vint *uart_ctrl;
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
		*uart_ctrl |= TIEN_MASK;
		break;
	case RCV:
		*uart_ctrl |= RIEN_MASK;
		break;
	}
	debug(DEBUG_UART_IRQ, "uart_ctrl = 0x%x, *uart_ctrl = 0x%x", uart_ctrl, *uart_ctrl);
}

void uart_device_disable(int channel, UART_IRQ_TYPE type)
{
	debug(DEBUG_UART_IRQ, "enter %s", "uart_device_disable");
	vint *uart_ctrl;
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
		*uart_ctrl &= ~TIEN_MASK;
		break;
	case RCV:
		*uart_ctrl &= ~RIEN_MASK;
		break;
	}
}

void uart_irq_handle(int channel, Kernel_state *ks)
{
    // check UART interrupt status
	debug(DEBUG_UART_IRQ, "enter %s", "uart_irq_handle"); 
    vint *uart_intr, *pdata;
    vint receive_event;
    vint transmit_event;

	switch (channel) {
        case COM1:
            uart_intr = (vint *) UART1_INTR;
            pdata = (vint *) UART1_DATA;
            receive_event = RCV_UART1_RDY;
            transmit_event = XMIT_UART1_RDY;
            break;
        case COM2:
            uart_intr = (vint *) UART2_INTR;
            pdata = (vint *) UART2_DATA;
            receive_event = RCV_UART2_RDY;
            transmit_event = XMIT_UART2_RDY;
            break;
	}
	debug(DEBUG_UART_IRQ, "channel = %d, *uart_intr = 0x%x", channel, *uart_intr);
	
	if (*uart_intr & uart_receive_irq_mask()) {
        //debug(SUBMISSION, "handle rcv interupt %s", "");
        // receive interrupt
        if (ks->blocked_on_event[receive_event]) {
            // notify events await on receive ready
            volatile Task_descriptor *td = ks->event_blocks[receive_event];
            ks->event_blocks[receive_event] = NULL;
            ks->blocked_on_event[receive_event] = 0;
            td->state = STATE_READY;
            // read the data
			char ch = *pdata; 
            td->retval = ch;
            debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>>>>>Wake up rcv notifier %d, received %d", td->tid, ch);
            insert_task(td, &(ks->ready_queue));
        }
    }
	if (*uart_intr & uart_transmit_irq_mask()) {
        debug(DEBUG_UART_IRQ, "handle xmit interrupt %s", "");
		// new transmit interrupt handling 
        if (ks->blocked_on_event[transmit_event]) {
            // notify events await on transmit ready
            Task_descriptor *td = ks->event_blocks[transmit_event];
            ks->event_blocks[transmit_event] = NULL;
            ks->blocked_on_event[transmit_event] = 0;
            td->state = STATE_READY;
            // turn off the XMIT interrupt
			uart_device_disable(channel, XMIT);
            // write the data
			/*debug(DEBUG_UART_IRQ, "!!!!!! write data %d", td->ch); */
			*pdata = td->ch;
            /*debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>>>>>Wake up xmit notifier %d, ", td->tid); */
            /*debug(SUBMISSION, "Wake up xmit notifier %d, ", td->tid); */
            insert_task(td, &(ks->ready_queue));
        }
	}
}
