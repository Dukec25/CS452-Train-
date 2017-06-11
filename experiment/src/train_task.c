#include <define.h>
#include <bwio.h>
#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <clock_server.h>
#include <user_functions.h>

static int is_exit = 0;

void start_up_task()
{
	cli_startup();
	initialize_switch();
	Exit();
}

void clock_task()
{
	int elapsed_tenth_sec = 0;
	// digital clock
	Clock clock;
	clock_init(&clock);

	while (!is_exit) {
		Delay(1);	// update every 10ms
		elapsed_tenth_sec++;
		clock_update(&clock, elapsed_tenth_sec);
		cli_update_clock(&clock);
	}
	Exit();
}

void sensor_task() {
	while (!is_exit) {
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
		if (c != (char)-1) {
			command_buffer[command_buffer_pos++] = c;
		}
		if (c == '\r') {
			// user hits ENTER
			newlines += 1;
			irq_nextline(newlines);
			// parse command
			Command cmd;
			int result = command_parse(command_buffer, &command_buffer_pos, &train_id, &train_speed, &cmd);
			if (result == 1) {
				// user hits 'q', exit
				is_exit = 1;
				break;
			} else if (result == 1) {
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
		Putc(COM2, c);
	}
	Exit();
}
