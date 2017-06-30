#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <kernel.h>
#include <name_server.h>
#include <ts7200.h>
#include <uart_irq.h>

void train_task_admin()
{
	Handshake kill_all = HANDSHAKE_AKG;
	Handshake kill_all_reply = HANDSHAKE_AKG;
	vint kill_all_addr = &kill_all;

	int idle_tid = Create(PRIOR_LOWEST, idle_task);
	debug(SUBMISSION, "created idle_task taskId = %d", idle_tid);
	Send(idle_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	cli_startup();
	bwputc(COM1, START); // switches won't work without start command
	irq_io_tasks_cluster();

    reverse_initialize_switch();
	initialize_switch();
	sensor_initialization();

	int cli_tid = Create(PRIOR_MEDIUM, cli_server);
	debug(SUBMISSION, "created cli_server taskId = %d", cli_tid);
	Send(cli_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	int train_tid = Create(PRIOR_MEDIUM, train_server);
	debug(SUBMISSION, "created train_server taskId = %d", train_tid);
	Send(train_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	int expected_num_exit = 3;
	int num_exit = 0;
	int exit_list[3];
	exit_list[0] = idle_tid;
	exit_list[1] = cli_tid;
	exit_list[2] = train_tid;
	while(num_exit < expected_num_exit) {
		Handshake exit_handshake;
		Handshake exit_reply = HANDSHAKE_AKG;
		int exit_tid;
		Receive(&exit_tid, &exit_handshake, sizeof(exit_handshake));
		Reply(exit_tid, &exit_reply, sizeof(exit_reply));
		if (exit_handshake == HANDSHAKE_SHUTDOWN) {
			if (exit_tid == exit_list[0]) {
				exit_list[0] = INVALID_TID;
				num_exit++;
			}
			else if (exit_tid == exit_list[1]) {
				exit_list[1] = INVALID_TID;
				num_exit++;
			}
			else if (exit_tid == exit_list[2]) {
				exit_list[2] = INVALID_TID;
				num_exit++;
			}
		}
	}
	
	Exit();
}

void idle_task()
{
	debug(DEBUG_UART_IRQ, "enter %s", "idle_task");

	Handshake kill_all_reply = HANDSHAKE_AKG;
	int train_task_admin_tid = INVALID_TID;
	vint kill_all_addr;
	Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
	Reply(train_task_admin_tid, &kill_all_reply, sizeof(kill_all_reply));
	Handshake *kill_all = kill_all_addr;

	uint32 tid = MyTid();

	int i, j = 0;
	vint *uart1_error = (vint *) UART1_ERROR;
	while (*kill_all != HANDSHAKE_SHUTDOWN) {

		if(*uart1_error & FE_MASK){
			debug(SUBMISSION, "%s", "frame error");
		}
		if(*uart1_error & PE_MASK){
			debug(SUBMISSION, "%s", "parity error");
		}
		if(*uart1_error & BE_MASK){
			debug(SUBMISSION, "%s", "break error");
		}
		if(*uart1_error & OE_MASK){
			debug(SUBMISSION, "%s", "overrun error");
		}
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	debug(DEBUG_TASK, "j = %d, tid =%d exiting", j, tid);
	Exit();
}
