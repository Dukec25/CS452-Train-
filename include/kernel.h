#include <define.h> 

typedef struct task_descriptor
{
    uint32 *stack_pointer; 
    heap_t *priority_queue;
    uint32 id;
    uint32 parent_id;
    task_state state;
    // link register 
    // SPSR
    // return value 
} task_descriptor;

enum task_state{
    STATE_ACTIVE, 
    STATE_ZOMBIE,
    STATE_READY
};

int create( int priority, void(*code) );

int MyTid();

int MyParentTid();
