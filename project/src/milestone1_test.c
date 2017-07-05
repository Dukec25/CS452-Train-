#include <debug.h>
#include <user_functions.h>
#include <train.h>
#include <train_task.h>

static uint32 speed_seed = 0;
static uint32 stop_seed = 0;

void milestone1_test()
{
	irq_debug(SUBMISSION, "%s", "running milestone1_test");

	int cli_server_tid = INVALID_TID;
	while(!(cli_server_tid > 0 && cli_server_tid < MAX_NUM_TASKS)) {
		cli_server_tid = WhoIs("CLI_SERVER");
	}
	
	const int train_id = 69;

	int i = 0;

	while(1) {
		irq_debug(SUBMISSION, "milestone1_test iteration %d", i);

		Handshake handshake = HANDSHAKE_AKG;

		Command tr_cmd;
		tr_cmd.type = TR;
		tr_cmd.arg0 = train_id;
		tr_cmd.arg1 = 14;

		irq_debug(SUBMISSION, "milestone1_test iteration %d: tr %d %d", i, tr_cmd.arg0, tr_cmd.arg1);
		Cli_request tr_cmd_request = get_train_command_request(tr_cmd);
		Send(cli_server_tid, &tr_cmd_request, sizeof(tr_cmd_request), &handshake, sizeof(handshake));

		Getc(COM2);

		Command park_cmd;
		park_cmd.type = PARK;
		park_cmd.arg0 = 'd';
		park_cmd.arg1 = 12;
		irq_debug(SUBMISSION, "milestone1_test iteration %d: park %c %d", i, park_cmd.arg0, park_cmd.arg1);

		Cli_request park_cmd_request = get_train_command_request(park_cmd);
		Send(cli_server_tid, &park_cmd_request, sizeof(park_cmd_request), &handshake, sizeof(handshake));

		Getc(COM2);
		i++;
	}

	debug(SUBMISSION, "%s", "milestone1_test exiting");
	Exit();
}
