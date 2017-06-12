#include <define.h>
#include <bwio.h>
#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <clock_server.h>
#include <user_functions.h>
#include <debug.h>

void train_task_startup()
{
	cli_startup();
	initialize_switch();
}

void clock_task()
{
	debug(SUBMISSION, "enter %s", "clock_task");
	vint elapsed_tenth_sec = 0;
	// digital clock
	Clock clock;
	clock_init(&clock);

	while (1) {
        debug(SUBMISSION, "%s", "before delay");
		Delay(10);	// update every 100ms
        debug(SUBMISSION, "%s", "after delay");
		elapsed_tenth_sec++;
		debug(DEBUG_UART_IRQ, "!!!!!!delayed time interval, elapsed_tenth_sec = %d", elapsed_tenth_sec);
        debug(SUBMISSION, "%s", "before clock_update");
        clock_update(&clock, elapsed_tenth_sec);
        cli_update_clock(&clock);
	}
	Exit();
}

void sensor_task() {
	while (1) {
		Delay(20);	// delay 200ms
		Putc(COM1, SENSOR_QUERY);
		int sensor_data[SENSOR_GROUPS];
		int group = 0;
		for (group = 0; group < SENSOR_GROUPS; group++) {
			sensor_data[group] = Getc(COM1);
		}
		for (group = 0; group < SENSOR_GROUPS; group++) {
			int id = 0;
			for (id = 0; id < SENSORS_PER_GROUP; id++) {
				if (sensor_data[group] & (0x1 << id)) {
					cli_update_sensor(group, id);
				}
			}
		}
	}
	Exit();
}

void train_task() {
	// command
	char command_buffer[COMMAND_SIZE];
	int command_buffer_pos = 0;
	char train_id = 0;
	char train_speed = 0;

	// console set up
	int newlines = 0;

	while(1) {
		// user I/O and send command to train
		char c = Getc(COM2);
		if (c == 'q') {
			// user hits 'q', exit
			debug(SUBMISSION, "%s", "user hits q, exiting");	
			break;
		}
		else if (c == '\r') {
			// user hits ENTER
			newlines += 1;
			irq_nextline(newlines);
			// parse command
			Command cmd;
			int result = command_parse(command_buffer, &command_buffer_pos, &train_id, &train_speed, &cmd);
			if (result != -1) {
				// user enters a valid command, handle command and sends command to train
				command_handle(&cmd);
				// update command line interface
				if (TR == cmd.type) {
					cli_update_train(cmd.arg0, cmd.arg1);
				} else if (SW == cmd.type) {
					cli_update_switch(cmd.arg0, cmd.arg1);
				}
			}
		}
		else {
			command_buffer[command_buffer_pos++] = c;
		}
		Putc(COM2, c);
	}
	Exit();
}
