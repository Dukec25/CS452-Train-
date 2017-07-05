#include <debug.h>
#include <kernel.h>
#include <time.h>
#include <irq.h>

#define IDLE_TASK	4

extern void asm_kernel_swiEntry();
extern void asm_kernel_hwiEntry();
extern void asm_init_kernel();
extern int asm_kernel_activate(Task_descriptor *td, int is_entry_from_hwi);
extern int asm_get_spsr();
extern int asm_get_sp();
extern int asm_get_fp();

int activate(Task_descriptor *td)
{
	td->state = STATE_ACTIVE;
	irq_debug(DEBUG_TRACE, "In activate tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, retval=0x%x, is_entry_from_hwi = 0x%x",
					td->tid, td->state, td->priority, td->sp, td->lr, td->retval, td->is_entry_from_hwi);
    /*irq_debug(SUBMISSION, "%x", td->lr);*/
	int is_entry_from_hwi = 0;
	if (td->is_entry_from_hwi == ENTER_FROM_HWI) {
		is_entry_from_hwi = td->is_entry_from_hwi;
	}
	td->is_entry_from_hwi = 0;
	return asm_kernel_activate(td, is_entry_from_hwi);
}

int main()
{
	// idle task measurement
	long long elapsed_time = 0;
	long long idle_task_time = 0;
	timer4_start();
	elapsed_time = timer4_read();

    bwsetspeed(COM1, 2400);
    bwsetfifo(COM1, OFF);
    bwsetspeed(COM2, 115200);
    bwsetfifo(COM2, OFF);

    asm volatile("MRC p15, 0, r2, c1, c0, 0");
    asm volatile("ORR r2, r2, #1<<12");
    asm volatile("ORR r2, r2, #1<<2");
    asm volatile("MCR p15, 0, r2, c1, c0, 0");

	// set up swi jump table 
	vint *swi_handle_entry = (vint*) 0x28;
	irq_debug(DEBUG_TRACE, "swi_handle_entry = 0x%x", swi_handle_entry);
	irq_debug(DEBUG_TRACE, "asm_kernel_swiEntry = 0x%x", asm_kernel_swiEntry);
	*swi_handle_entry = (vint) (asm_kernel_swiEntry + 0x218000);
	irq_debug(DEBUG_TRACE, "swi_handle_entry = 0x%x", *swi_handle_entry);

	// set up hwi jump table
	vint *hwi_handle_entry = (vint*) 0x38;
	irq_debug(DEBUG_UART_IRQ, "hwi_handle_entry = 0x%x", hwi_handle_entry);
	irq_debug(DEBUG_UART_IRQ, "asm_kernel_hwiEntry = 0x%x", asm_kernel_hwiEntry);
	*hwi_handle_entry = (vint) (asm_kernel_hwiEntry + 0x218000);
	irq_debug(DEBUG_UART_IRQ, "hwi_handle_entry = 0x%x", *hwi_handle_entry);

	Kernel_state ks;
	ks_initialize(&ks);

	uint8 tid = 0;
	td_intialize(first_task, &ks, tid++, INVALID_TID, PRIOR_MEDIUM);

	// enable irq
    irq_disable(); // reset the state in case previous one messed it up
    irq_enable();

	volatile Task_descriptor *td = NULL;
	vint is_entry_from_hwi = 0;
    vint cts_send = -1;
	while(ks.ready_queue.mask != 0) {
			irq_debug(DEBUG_TRACE, "mask =%d", ks.ready_queue.mask);
			td = schedule(&ks);
		
			// idle task time measurement before exit kernel
			if (td->tid == IDLE_TASK) {
				idle_task_time -= timer4_read();
			}
			irq_debug(DEBUG_IRQ, "tid = %d, state = %d, priority = %d, sp = 0x%x, lr = 0x%x, next_task = %d",
					td->tid, td->state, td->priority, td->sp, td->lr,
					td->next_task ? td->next_task->tid : INVALID_TID);

			// retrieve lr and retrieve syscall request type
			vint cur_lr = activate(td);
			irq_debug(DEBUG_IRQ, "td %d get back into kernel again, cur_lr = 0x%x", td->tid, cur_lr);

			// idle task time measurement after enter kernel
			if (td->tid == IDLE_TASK) {
				idle_task_time += timer4_read();
			}

			if (cur_lr & HWI_MASK) {
				// hwi entry bit is set, entered from hwi
				cur_lr = cur_lr & ~(HWI_MASK);
				is_entry_from_hwi = 1;
				td->is_entry_from_hwi = ENTER_FROM_HWI;
			}

			update_td(td, cur_lr);
			
			if (is_entry_from_hwi) {
			//	irq_debug(DEBUG_UART_IRQ, ">>>>>>>>>>>is_entry_from_hwi = %d, start irq handling", is_entry_from_hwi);
				irq_handle(&ks, &cts_send);
				is_entry_from_hwi = 0;
				continue;
			}
 
			uint32 immed_24 = *((vint *)((int) td->lr - 4)) & ~(0xff000000);
			uint32 req = immed_24 & 0xfff;
			uint32 argc = (immed_24 >> 12) & 0xfff;
			irq_debug(DEBUG_IRQ, "swi get back into kernel again, immed_24 = 0x%x, req = %d, argc = %d", immed_24, req, argc);

			vint arg0 = *((vint*) ((int) td->sp + 0));
			uint32 arg1 = *((vint*) ((int) td->sp + 4));
            uint32 arg2 = *((vint*) ((int) td->sp + 8));
            uint32 arg3 = *((vint*) ((int) td->sp + 12 )); 
            uint32 arg4 = *((vint*) ((int) td->sp + 4));
			irq_debug(DEBUG_TRACE, "arg0 = %d, arg1 = %d", arg0, arg1);

            // uart1_irq_soft_clear();
			switch(req){
				case 1:		
					k_create(td, &ks, (void (*) ()) arg1, tid++, arg0);
					break;
				case 2:
					k_pass(td, &ks);
					break;
				case 3:
					k_my_tid(td, &ks);
					break;
				case 4:
					k_exit(td, &ks);
					break;
				case 5:
					k_my_parent_tid(td, &ks);
					break;
                case 6:
                    k_send(arg0, arg1, arg2, arg3, arg4, td, &ks);
                    break;
                case 7:
                    k_receive(arg0, arg1, arg2, td, &ks);
                    break;
                case 8:
                    k_reply(arg0, arg1, arg2, td, &ks);
                    break;
				case 9:
                    /*irq_debug(SUBMISSION, "1%d", td->tid);*/
					k_await_event(arg0, arg1, td, &ks);
					break;
                case 10:
                    ks.ready_queue.mask = 0;
                    break;
			}
	}
    irq_disable();

	// idle task measurement
	elapsed_time = timer4_read() - elapsed_time;
	timer4_stop();
//	irq_debug(SUBMISSION, "idle task running time = %dus", idle_task_time);
//	irq_debug(SUBMISSION, "elapsed time = %dus", elapsed_time);
//	long long fraction = (idle_task_time * 100) / elapsed_time;
//	irq_debug(SUBMISSION, "idle task took %d percent of total running time", fraction);
	return 0;
}
