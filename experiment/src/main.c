#include <bwio.h>
#include <kernel.h>
#include <define.h>

extern int asm_print_sp();
extern void asm_kernel_swiEntry();
extern void asm_init_kernel();
extern void asm_kernel_activate(task_descriptor *td);

void print_td(task_descriptor *td)
{
	int sp = asm_print_sp();
	bwprintf(COM2, "%s:%d sp = %x\n", __FILE__, __LINE__, sp);
	bwprintf(COM2, "%s:%d td->id = %d, td->state = %d\n", __FILE__, __LINE__, td->id, td->state);
	bwprintf(COM2, "%s:%d td->sp = 0x%x, td->lr = 0x%x\n", __FILE__, __LINE__, td->sp, td->lr);
}

void print_ks(kernel_state *ks)
{
	int sp = asm_print_sp();
	bwprintf(COM2, "%s:%d sp = %x\n", __FILE__, __LINE__, sp);
	bwprintf(COM2, "%s:%d ks->u_sp = %d, ks->u_lr = %d\n", __FILE__, __LINE__, ks->u_sp, ks->u_lr);
}

void intialize(task_descriptor *td, vint **pavailable_memeory_ptr, void (*task)(), heap_t *pready_queue)
{
    	td->id = 1;
	td->parent_id = 0;
    	td->state = STATE_READY;
	td->spsr = 16;
	//assign memory to the first task
	td->sp = *pavailable_memeory_ptr; 
	*pavailable_memeory_ptr += TASK_SIZE;
	// assign lr to point to the function pointer
	td->lr = (vint *)task;
	bwprintf(COM2, "%s:%d td->id = %d, td->state = %d\n", __FILE__, __LINE__, td->id, td->state);
	bwprintf(COM2, "%s:%d td->sp = 0x%x, td->lr = 0x%x\n", __FILE__, __LINE__, td->sp, td->lr);
	heap_insert(pready_queue, PRIOR_MEDIUM, td);
	td->priority_queue = pready_queue;
}

void handle(task_descriptor *active, void *req) {}

void kernel_swi_entry(){
    asm_kernel_swiEntry();
}

task_descriptor *schedule(heap_t *ready_queue) {
	node_t head;
	heap_delete(ready_queue, &head);
	return head.data;
}

void activate(task_descriptor *td, kernel_state *ks) {
	bwprintf(COM2, "line %d, in activate\n", __LINE__);
	ks->u_sp = td->sp;
	ks->u_lr = td->lr;
	ks->u_spsr = td->spsr;
	bwprintf(COM2, "%s:%d ks->u_sp = 0x%x, ks->u_lr = 0x%x\n", __FILE__, __LINE__, ks->u_sp, ks->u_lr);	
	//fifo_put(&(ks->active_tasks), td);
	asm_kernel_activate(td);
}

int main()
{
        vint *swi_handle_entry = (vint*)0x28;
        bwprintf(COM2, "line %d, swi_handle_entry = 0x%x\n", __LINE__, swi_handle_entry);
        bwprintf(COM2, "line %d, asm_kernel_swiEntry = 0x%x\n", __LINE__, asm_kernel_swiEntry);
        bwprintf(COM2, "line %d, asm_init_kernel = 0x%x\n", __LINE__, asm_init_kernel);
        *swi_handle_entry = (vint*)(asm_kernel_swiEntry + 0x218000);
        bwprintf(COM2, "line %d, swi_handle_entry = 0x%x\n", __LINE__, *swi_handle_entry);

	heap_t ready_queue;
	node_t data[NUM_TASK];
	ready_queue = heap_init(data, NUM_TASK);

	vint *available_memeory_ptr = (vint*) TASK_START_LOCATION;
	task_descriptor task1_td;
	// init_kernel is the first task
	intialize(&task1_td, &available_memeory_ptr, init_kernel, &ready_queue);

	fifo_t active_tasks;
	kernel_state ks;
	ks.priority_queue = &ready_queue;
	ks.active_tasks = &active_tasks;
	register int *rb asm("lr");
	ks.rb_lr = rb;
	for(;;) {
		// scheduling td = priorityQueue.pull()
		task_descriptor *td = schedule(&ready_queue);
		bwprintf(COM2, "%s:%d td->id = %d, td->state = %d\n", __FILE__, __LINE__, td->id, td->state);
		bwprintf(COM2, "%s:%d td->sp = 0x%x, td->lr = 0x%x\n", __FILE__, __LINE__, td->sp, td->lr);
		// active(td);
		activate(td, &ks);
          	// handle()
	}
        // init_kernel();
	return 0;
}

/*
void first(void) {
	bwputstr(COM2, "In user mode\n");
	while(1);
}
int main(void) {
	bwputstr(COM2, "Starting\n");
	activate();

	while(1); // We can't exit, there's nowhere to go
	return 0;
}
*/
