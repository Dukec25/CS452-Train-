#include <irq.h>
#include <debug.h>
#include <kernel.h>

void irq_enable()
{
	debug(DEBUG_IRQ, "enter %s", "irq_enable");
	vint *vic2_int_sel = (vint *) VIC2_INT_SEL;
	*vic2_int_sel &= 0x0;	// IRQ
	timer_irq_enable();
#ifdef SOFT
	timer_irq_soft();
#else
	timer_enable();
#endif
}

void irq_disable()
{
	timer_irq_disable();
}

void irq_handle(Kernel_state *ks)
{
	timer_irq_handle(ks);
}

static uint32 timer_irq_mask()
{
	return 0x1 << (TIMER_UNDER_FLOW_INTERRUPT - 32);
}

void timer_enable()
{
	const uint32 load_val = TIMER_REQUENCY * TICK / SEC;
	debug(DEBUG_IRQ, "enter timer_enable, load_val = %d", load_val);

	vint *timer_ctrl = (vint *) TIMER_CTRL;
	*timer_ctrl = (*timer_ctrl) ^ ENABLE_MASK;
	vint *timer_ldr = (vint *) TIMER_LDR;    
	*timer_ldr = load_val;

	*timer_ctrl = ENABLE_MASK | CLKSEL_MASK | MODE_MASK;	
}

void timer_irq_enable()
{
	debug(DEBUG_IRQ, "enter %s", "timer_irq_enable");
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl |= timer_irq_mask();
//	*vic2_int_enbl |= 0xFFFFFFFF;
}

void timer_irq_soft()
{
	debug(DEBUG_IRQ, "enter %s", "timer_irq_soft_enable");
	vint *vic2_soft_int = (vint *) VIC2_SOFT_INT;
	*vic2_soft_int |= timer_irq_mask();
//	*vic2_soft_int |= 0xFFFFFFFF;
}

void timer_irq_disable()
{
	vint *vic2_int_enbl = (vint *) VIC2_INT_ENBL;
	*vic2_int_enbl &= ~timer_irq_mask();
	vint *vic_int_enbl_clr = (vint *) VIC2_INT_ENBL_CLR;
	*vic_int_enbl_clr |= (0x1 << (TIMER_UNDER_FLOW_INTERRUPT - 32));
}

void timer_irq_handle(Kernel_state *ks)
{
	debug(DEBUG_IRQ, "enter %s, reached 10 ms", timer_irq_handle);
	vint *vic_soft_int_clr = (vint *) VIC2_SOFT_INT_CLR;
	*vic_soft_int_clr |= (0x1 << (TIMER_UNDER_FLOW_INTERRUPT - 32));
	if (ks->blocked_on_event[0]) {
		// notify events await on timer
		Task_descriptor *td = pull_highest_priority_task(&ks->event_blocks[0]);
		ks->blocked_on_event[0]--;
        insert_task(td, &(ks->ready_queue));
	}
}
