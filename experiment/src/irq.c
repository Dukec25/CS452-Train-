#include <irq.h>
#include <debug.h>
#include <kernel.h>

static uint32 timer3_irq_mask()
{
	return 0x1 << (TIMER3_UNDER_FLOW_INTERRUPT - 33);
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
	debug(DEBUG_IRQ, "enter %s", "irq_handle");
	vint *vic2_irq_status = (vint *) VIC2_IRQ_STATUS;
	debug(DEBUG_IRQ, "*vic2_irq_status = 0x%x", *vic2_irq_status);
	if (*vic2_irq_status & timer3_irq_mask()) {
		timer3_irq_handle(ks);
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
	timer3_clear(); // is this needed to clear the timer manually
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
