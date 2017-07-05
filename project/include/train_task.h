#ifndef __TRAIN_TASK__
#define __TRAIN_TASK__

void train_task_admin();
void idle_task();

typedef enum Handshake {
	HANDSHAKE_AKG,
	HANDSHAKE_SHUTDOWN
} Handshake;

#endif // __TRAIN_TASK__
