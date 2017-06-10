#include <irq.h>
#include <debug.h>
#include <kernel.h>
#include <uart_irq.h>

static uint32 timer3_irq_mask()
{
	return 0x1 << (TIMER3_UNDER_FLOW_INTERRUPT - 33);
}

static uint32 uart_modem_irq_mask()
{
    return 0x1;
}

static uint32 uart_receive_irq_mask()
{
    return 0x1 << 1;
}

static uint32 uart_transmit_irq_mask()
{
    return 0x1 << 2;
}

void irq_enable()
{
	debug(DEBUG_IRQ, "enter %s", "irq_enable");
	vint *vic2_int_sel = (vint *) VIC2_INT_SEL;
	*vic2_int_sel &= 0x0;	// interrupt type = IRQ
//	timer3_irq_enable();
//	timer3_enable();
	// UART2 RCV
//	uart_irq_enable(COM1);
	uart_irq_enable(COM2);
	uart_device_enable(COM2, RCV);
}

void irq_disable()
{
	timer3_irq_disable();
}

void irq_handle(Kernel_state *ks)
{
	debug(DEBUG_UART_IRQ, "enter %s", "irq_handle");
	vint *vic2_irq_status = (vint *) VIC2_IRQ_STATUS;
	debug(DEBUG_UART_IRQ, "*vic2_irq_status = 0x%x", *vic2_irq_status);
	if ((*vic2_irq_status & timer3_irq_mask()) != 0) {
		timer3_irq_handle(ks);
    }
	// else if doesn't work for some reason
	if ((*vic2_irq_status & uart_irq_mask(COM1)) != 0) {
        debug(DEBUG_UART_IRQ, "handle uart interupt %s", "UART1");
        uart_irq_handle(COM1, ks);
    }
	if ((*vic2_irq_status & uart_irq_mask(COM2)) != 0) {
        debug(DEBUG_UART_IRQ, "handle uart interupt %s", "UART2");
        uart_irq_handle(COM2, ks);
    }
}

void timer3_enable()
{
	const uint32 load_val = TIMER3_REQUENCY * TICK / SEC;	
	debug(DEBUG_IRQ, "enter timer3_enable, load_val = %d", load_val);

	vint *timer3_ctrl = (vint *) TIMER3_CTRL;
	*timer3_ctrl = (*timer3_ctrl) ^ ENABLE_MASK;
	vint *timer3_ldr = (vint *) TIMER3_LDR;    
	*timer3_ldr = load_val;

	*timer3_ctrl = ENABLE_MASK | CLKSEL_MASK | MODE_MASK;	
}

void timer3_clear()
{
	vint *timer3_clr = (vint *) TIMER3_CLR;
	*timer3_clr = 0xFFFFFFFF;	
}

void timer3_irq_enable()
{
	debug(DEBUG_IRQ, "enter %s", "timer3_irq_enable");
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl |= timer3_irq_mask();
}

void timer3_irq_soft()
{
	debug(DEBUG_IRQ, "enter %s", "timer3_irq_soft");
	vint *vic2_soft_int = (vint *) VIC2_SOFT_INT;
	*vic2_soft_int |= timer3_irq_mask();
}

void timer3_irq_disable()
{
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl &= ~timer3_irq_mask();
	vint *vic_int_enbl_clr = (vint *) VIC2_INT_ENBL_CLR;
	*vic_int_enbl_clr |= timer3_irq_mask();
}

void timer3_irq_soft_clear()
{
	debug(DEBUG_IRQ, "enter %s", "timer3_irq_soft_clear");
	vint *vic2_soft_int_clr = (vint *) VIC2_SOFT_INT_CLR;
	*vic2_soft_int_clr |= 0xFFFFFFFF;
}

void timer3_irq_handle(Kernel_state *ks)
{
	debug(DEBUG_IRQ, ">>>>>>>>>>>>>>>>>>>>enter %s, reached time limit", "timer3_irq_handle");
	timer3_clear();
	if (ks->blocked_on_event[0]) {
		// notify events await on timer
		volatile Task_descriptor *td = ks->event_blocks[0];
		ks->event_blocks[0] = NULL;
		ks->blocked_on_event[0] = 0;
        td->state = STATE_READY;
		debug(DEBUG_IRQ, ">>>>>>>>>>>>>>>>>>>>>Wake up task %d, ks->blocked_on_event[0] = %d", td->tid, ks->blocked_on_event[0]);
        insert_task(td, &(ks->ready_queue));
	}
	debug(DEBUG_IRQ, ">>>>>>>>>>>>>>>>>>>> %s, no task to get awaked", "timer3_irq_handle");
}

void uart_irq_handle(int channel, Kernel_state *ks){
    // check UART interrupt status
	debug(DEBUG_UART_IRQ, "enter %s", "uart_irq_handle"); 
    vint *uart_intr, *pdata;
    vint receive_event;
    vint transmit_event;

	switch (channel) {
	case COM1:
		uart_intr = (vint *) UART1_INTR;
        *pdata = (vint *) UART1_DATA;
        receive_event = RCV_UART1_RDY;
        transmit_event = XMIT_UART1_RDY;
		break;
	case COM2:
		uart_intr = (vint *) UART2_INTR;
        *pdata = (vint *) UART2_DATA;
        receive_event = RCV_UART2_RDY;
        transmit_event = XMIT_UART2_RDY;
		break;
	}
	debug(DEBUG_UART_IRQ, "channel = %d, *uart_intr = 0x%x", channel, *uart_intr); 
	
	if (*uart_intr & uart_receive_irq_mask()) {
        debug(DEBUG_UART_IRQ, "handle rcv interupt %s", "");
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
	else if (*uart_intr & uart_transmit_irq_mask()) {
		// new transmit interrupt handling 
        if (ks->blocked_on_event[transmit_event]) {
            // notify events await on transmit ready
            volatile Task_descriptor *td = ks->event_blocks[transmit_event];
            ks->event_blocks[transmit_event] = NULL;
            ks->blocked_on_event[transmit_event] = 0;
            td->state = STATE_READY;
            // turn off the XMIT interrupt
			uart_device_disable(channel, XMIT);
            // write the data 
            *pdata = td->ch;
            debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>>>>>Wake up xmit notifier %d, ", td->tid); 
            insert_task(td, &(ks->ready_queue));
        }
	}
}
