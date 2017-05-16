#include <bwio.h>
#include <kernel.h>
#include <define.h>

extern int asm_print_sp();
extern void asm_kernel_swiEntry();
extern void asm_init_kernel();


void print_td(task_descriptor *td)
{
	int sp = asm_print_sp();
	bwprintf(COM2, "sp = %x\n", sp);
	bwprintf(COM2, "td->id = %d, td->state = %d\n", td->id, td->state);
}

heap_t intialize(task_descriptor *td, vint **pavailable_memeory_ptr)
{
	node_t data[NUM_TASK]; 
	heap_t ready_queue = heap_init(data, NUM_TASK);
    	td->id = 1;
	td->parent_id = 0;
    	td->state = STATE_READY;
	td->stack_pointer = *pavailable_memeory_ptr; 
	*pavailable_memeory_ptr += TASK_SIZE; //assign memory to the first task
	heap_insert(&ready_queue, PRIOR_MEDIUM, td);
	print_td(&td);
	return ready_queue;
}

void kerent() {}

void kerxit(task_descriptor *active, void *req)
{
	bwprintf(COM2, "kerxit.c: Hello.\n\r");
	bwprintf(COM2, "kerxit.c: Activating.\n\r");
	kerent();
	bwprintf(COM2, "kerxit.c: Good-bye.\n\r");
}

void handle(task_descriptor *active, void *req) {}

void kernel_swi_entry(){
    asm_kernel_swiEntry();
}

int main()
{
        vint *swi_handle_entry = (vint*)0x28;
        bwprintf(COM2, "line %d, swi_handle_entry = 0x%x\n", __LINE__, swi_handle_entry);
        bwprintf(COM2, "line %d, asm_kernel_swiEntry = 0x%x\n", __LINE__, asm_kernel_swiEntry);
        bwprintf(COM2, "line %d, asm_init_kernel = 0x%x\n", __LINE__, asm_init_kernel);
        *swi_handle_entry = (vint*)(asm_kernel_swiEntry+0x218000);
        bwprintf(COM2, "line %d, swi_handle_entry = 0x%x\n", __LINE__, *swi_handle_entry);
        /*bwprintf(COM2, "line %d, pc = %x\n", __current_pc());*/
	/*vint *available_memeory_ptr = (vint*) TASK_START_LOCATION;*/
	/*task_descriptor task1_td;*/
	/*heap_t ready_queue = intialize(&task1_td, &available_memeory_ptr);*/
	/*{*/
		/*node_t del;*/
		/*task_descriptor *active;*/
		/*void *req;*/
                /*heap_delete(&ready_queue, &del);*/
//		active = (task_descriptor *)del.data;
//		kerxit(active, req);
//		handle(active, req); 
	/*}*/
        init_kernel();
	return 0;
}
