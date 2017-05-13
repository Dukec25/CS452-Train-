#include <kernel.h> 
#include <heap.h>

int main()
{
    // initialization of priority queue 
    node_t data[NUM_TASK]; 
    heap_t ready_queue = heap_init(data, NUM_TASK);
    uint32 *available_memeory_ptr = (uint32*)(TASK_START_LOCATION);
    // probably also need a block queue 

    // create first td and fill in value
    task_descriptor task1_td;
    task1_td.id = 1;
    task1_td.parentId = 0; // which means the first task get created 
    task1_td.stack_pointer = available_memeory_ptr; 
    *available_memeory_ptr += TASK_SIZE; //assign memory to the first task
    task1_td.state = STATE_READY;
    //TODO more initialization for td 1

    heap_insert(&ready_queue, PRIOR_MEDIUM, &task1_td);

    for(;;){
	node_t task, del;
        task = heap_root(&ready_queue);
        heap_delete(&task, &del);
        // active the task, do context switch, leave this tomorrow 
    }
}
