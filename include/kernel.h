#include <heap.h>
#include <define.h> 

enum task_state{
    STATE_ACTIVE, 
    STATE_ZOMBIE,
    STATE_READY
};

typedef struct task_descriptor
{
    vint *stack_pointer; 
    heap_t *priority_queue;
    uint32 id;
    uint32 parent_id;
    enum task_state state;
    // link register 
    // SPSR
    // return value 
} task_descriptor;

int create( int priority, void(*code) );

int MyTid();

int MyParentTid();
