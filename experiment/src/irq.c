#include <irq.h>
#include <debug.h>
#include <kernel.h>
#include <uart_irq.h>

static uint32 timer3_irq_mask()
{
	return 0x1 << (TIMER3_UNDER_FLOW_INTERRUPT - 33);
}

static uint32 uart1_irq_mask()
{
	return 0x1 << (UART1_GENERAL_INTERRUPT - 33);
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
	timer3_irq_enable();
	timer3_enable();
}

void irq_disable()
{
	timer3_irq_disable();
}

void irq_handle(Kernel_state *ks)
{
	debug(DEBUG_UART_IRQ, "enter %s", "irq_handle");
	vint *vic2_irq_status = (vint *) VIC2_IRQ_STATUS;
	debug(DEBUG_UART_IRQ, "*vic2_irq_status = 0x%x, uart1_irq_mask = 0x%x", *vic2_irq_status, uart1_irq_mask());
//	if (*vic2_irq_status & timer3_irq_mask() != 0) {
//		timer3_irq_handle(ks);
//    }
//	else if (*vic2_irq_status & uart1_irq_mask() != 0) {
        debug(DEBUG_UART_IRQ, "handle uart interupt %s", "");
        uart1_irq_handle(ks);
//    }
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

void uart1_irq_handle(Kernel_state *ks){
    // check UART interrupt status
	debug(DEBUG_UART_IRQ, "enter %s", "uart1_irq_handle"); 
    vint *uart1_intr = (vint *)UART1_INTR;
	debug(DEBUG_UART_IRQ, "*uart1_intr = 0x%x", *uart1_intr); 
/*    if (*uart1_intr & uart_receive_irq_mask()) {
        debug(DEBUG_UART_IRQ, "handle rcv interupt %s", "");
        // receive interrupt
        if (ks->blocked_on_event[RCV_RDY]) {
            // notify events await on receive ready
            volatile Task_descriptor *td = ks->event_blocks[RCV_RDY];
            ks->event_blocks[RCV_RDY] = NULL;
            ks->blocked_on_event[RCV_RDY] = 0;
            td->state = STATE_READY;
            vint *pdata = (vint *) UART1_DATA;
            td->retval = *pdata;
            debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>>>>>Wake up rcv notifier %d, ", td->tid);
            insert_task(td, &(ks->ready_queue));
        }
    }
	else if (*uart1_intr & uart_transmit_irq_mask()) {
*/      
		debug(DEBUG_UART_IRQ, "handle xmit interupt %s", "");
//		uart1_irq_soft_clear();
        if (ks->blocked_on_event[XMIT_RDY]) {
            // notify events await on transmit ready
            volatile Task_descriptor *td = ks->event_blocks[XMIT_RDY];
            ks->event_blocks[XMIT_RDY] = NULL;
            ks->blocked_on_event[XMIT_RDY] = 0;
            td->state = STATE_READY;
            debug(DEBUG_UART_IRQ, ">>>>>>>>>>>>>>>>>>>>>Wake up xmit notifier %d, ", td->tid);
            insert_task(td, &(ks->ready_queue));
        }
//    }
}
