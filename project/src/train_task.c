#include <train_task.h>
#include <debug.h>
#include <log.h>
#include <user_functions.h>
#include <ts7200.h>
#include <uart_irq.h>

void train_task_admin()
{
	Handshake kill_all = HANDSHAKE_AKG;
	Handshake kill_all_reply = HANDSHAKE_AKG;
	vint kill_all_addr = &kill_all;

	int idle_tid = Create(PRIOR_LOWEST, idle_task);
	irq_debug(SUBMISSION, "created idle_task taskId = %d", idle_tid);
	Send(idle_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	cli_startup();
	bwputc(COM1, START); // switches won't work without start command
	irq_io_tasks_cluster();

    reverse_initialize_switch();
	initialize_switch();
	sensor_initialization();

	int cli_tid = Create(PRIOR_MEDIUM, cli_server);
	irq_debug(SUBMISSION, "created cli_server taskId = %d", cli_tid);
	Send(cli_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	int train_tid = Create(PRIOR_MEDIUM, train_server);
	irq_debug(SUBMISSION, "created train_server taskId = %d", train_tid);
	Send(train_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	int train_command_courier_tid = Create(PRIOR_MEDIUM, train_command_courier);
	Send(train_command_courier_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

	int cli_request_courier_tid = Create(PRIOR_MEDIUM, cli_request_courier);
	Send(cli_request_courier_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));

    int train_to_park_courier_tid = Create(PRIOR_MEDIUM, train_to_park_courier);
    Send(train_to_park_courier_tid, &kill_all_addr, sizeof(kill_all_addr), &kill_all_reply, sizeof(kill_all_reply));
  
	/*int tid = Create(PRIOR_MEDIUM, 	milestone1_test);*/

	int expected_num_exit = 5;
	int num_exit = 0;
	int exit_list[5];
	exit_list[0] = idle_tid;
	exit_list[1] = cli_tid;
	exit_list[2] = train_tid;
	exit_list[3] = train_command_courier_tid;
	exit_list[4] = cli_request_courier_tid;
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
			else if (exit_tid == exit_list[3]) {
				exit_list[3] = INVALID_TID;
				num_exit++;
			}
			else if (exit_tid == exit_list[4]) {
				exit_list[4] = INVALID_TID;
				num_exit++;
			}
		}
	}
	
	Exit();
}

void idle_task()
{
	irq_debug(DEBUG_UART_IRQ, "enter %s", "idle_task");

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
			irq_debug(SUBMISSION, "%s", "frame error");
		}
		if(*uart1_error & PE_MASK){
			irq_debug(SUBMISSION, "%s", "parity error");
		}
		if(*uart1_error & BE_MASK){
			irq_debug(SUBMISSION, "%s", "break error");
		}
		if(*uart1_error & OE_MASK){
			irq_debug(SUBMISSION, "%s", "overrun error");
		}
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	irq_debug(DEBUG_TASK, "j = %d, tid =%d exiting", j, tid);
	Exit();
}

void train_command_courier()
{
	Handshake kill_all_reply = HANDSHAKE_AKG;
	int train_task_admin_tid = INVALID_TID;
	vint kill_all_addr;
	Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
	Reply(train_task_admin_tid, &kill_all_reply, sizeof(kill_all_reply));
	Handshake *kill_all = kill_all_addr;

	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	int train_server_tid = INVALID_TID;
	while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
		train_server_tid = WhoIs("TRAIN_SERVER");
	}

	Handshake handshake = HANDSHAKE_AKG;
	while (*kill_all != HANDSHAKE_SHUTDOWN) {
		Cli_request cli_server_msg;
		cli_server_msg.type = CLI_WANT_COMMAND;
		TS_request train_server_msg;
		Send(cli_server_tid, &cli_server_msg, sizeof(cli_server_msg), &train_server_msg, sizeof(train_server_msg));
		/*if (train_server_msg.type != CLI_NULL) irq_debug(SUBMISSION, "train_command_courier send msg %d", train_server_msg.type); */
		Send(train_server_tid, &train_server_msg, sizeof(train_server_msg), &handshake, sizeof(handshake)); 
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	Exit();
}

void cli_request_courier()
{
	Handshake kill_all_reply = HANDSHAKE_AKG;
	int train_task_admin_tid = INVALID_TID;
	vint kill_all_addr;
	Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
	Reply(train_task_admin_tid, &kill_all_reply, sizeof(kill_all_reply));
	Handshake *kill_all = kill_all_addr;

	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	int train_server_tid = INVALID_TID;
	while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
		train_server_tid = WhoIs("TRAIN_SERVER");
	}

	Handshake handshake = HANDSHAKE_AKG;
	while (*kill_all != HANDSHAKE_SHUTDOWN) {
		TS_request train_server_msg;
		train_server_msg.type = TS_WANT_CLI_REQ;
		Cli_request cli_server_msg;
		Send(train_server_tid, &train_server_msg, sizeof(train_server_msg), &cli_server_msg, sizeof(cli_server_msg));
		//if (cli_server_msg.type != TS_NULL) irq_debug(SUBMISSION, "cli_request_courier send msg %d", cli_server_msg.type); 
		Send(cli_server_tid, &cli_server_msg, sizeof(cli_server_msg), &handshake, sizeof(handshake)); 
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	Exit();
}

void train_to_park_courier()
{
    Handshake kill_all_reply = HANDSHAKE_AKG;
    int train_task_admin_tid = INVALID_TID;
    vint kill_all_addr;
    Receive(&train_task_admin_tid, &kill_all_addr, sizeof(kill_all_addr));
    Reply(train_task_admin_tid, &kill_all_reply, sizeof(kill_all_reply));
    Handshake *kill_all = kill_all_addr;

    int train_server_tid = INVALID_TID;
    while(!(train_server_tid > 0 && train_server_tid < MAX_NUM_TASKS)) {
        train_server_tid = WhoIs("TRAIN_SERVER");
    }
    int park_server_tid = INVALID_TID;
    while(!(park_server_tid > 0 && park_server_tid < MAX_NUM_TASKS)) {
        park_server_tid = WhoIs("PARK_SERVER");
    }

    Handshake handshake = HANDSHAKE_AKG;
    bwprintf(COM2, "train_to_park_courier get trigger");
    while (*kill_all != HANDSHAKE_SHUTDOWN) {
        TS_request train_server_msg;
        train_server_msg.type = TS_TRAIN_TO_PARK_REQ;
        Park_request park_req;
        Send(train_server_tid, &train_server_msg, sizeof(train_server_msg), &park_req, sizeof(park_req));
        Send(park_server_tid, &park_req, sizeof(park_req), &handshake, sizeof(handshake)); 
    }

    Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
    Handshake exit_reply;
    Send(train_task_admin_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
    Exit();
}
