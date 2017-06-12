#include <define.h>
#include <bwio.h>
#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <clock_server.h>
#include <user_functions.h>
#include <debug.h>
#include <kernel.h>
#include <name_server.h>

void train_task_startup()
{
	cli_startup();
	initialize_switch();
}

void clock_task()
{
	int command_buffer_addr = 0;
	int train_task_tid = 0;
	Receive(&train_task_tid, &command_buffer_addr, sizeof(command_buffer_addr));
	Reply(train_task_tid, 0, 0);
	Command_buffer * command_buffer = (Command_buffer *) command_buffer_addr;

	vint elapsed_tenth_sec = 0;
	// digital clock
	Clock clock;
	clock_init(&clock);

	while (1) {
		Delay(10);	// update every 100ms
		elapsed_tenth_sec++;
		debug(DEBUG_UART_IRQ, "!!!!!!delayed time interval, elapsed_tenth_sec = %d", elapsed_tenth_sec);
        clock_update(&clock, elapsed_tenth_sec);
        cli_update_clock(&clock);
		cli_user_input(command_buffer);
	}
	Exit();
}

void sensor_task() {
	int result = RegisterAs("train_task");

	int updates = 0;
	while (1) {
		Delay(20);	// delay 200ms
		int sensor_data_lo[SENSOR_GROUPS];
		int sensor_data_hi[SENSOR_GROUPS];
		int group = 0;
		for (group = 0; group < SENSOR_GROUPS; group++) {
			Putc(COM1, SENSOR_QUERY_BASE + group);
			sensor_data_lo[group] = Getc(COM1);
		//	irq_printf(COM2, "sensor_data_lo[%d] = %d\r\n", group, sensor_data_lo[group]);
			sensor_data_hi[group] = Getc(COM1);
		//	irq_printf(COM2, "sensor_data_hi[%d] = %d\r\n", group, sensor_data_hi[group]);
		}

		for (group = 0; group < SENSOR_GROUPS; group++) {
			int id = 0;
			for (id = 0; id < SENSORS_PER_GROUP / 2; id++) {
				if (sensor_data_lo[group] & (0x1 << id)) {
					updates++;
					cli_update_sensor(group, id, updates);
				}
				if (sensor_data_hi[group] & (0x1 << id)) {
					updates++;
					cli_update_sensor(group, SENSORS_PER_GROUP / 2 + id, updates);
				}
			}
		}
	}
	Exit();
}

void train_task() {
	// command
	Command_buffer command_buffer;
	command_buffer.pos = 0;
	char train_id = 0;
	char train_speed = 0;

	int result = RegisterAs("train_task");
/*
    int clock_task_tid = Create(PRIOR_MEDIUM, clock_task);

	int command_buffer_addr = (int) &command_buffer;
	Send(clock_task_tid, &command_buffer_addr, sizeof(command_buffer_addr), 0, 0);
*/
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
			// parse command
			Command cmd;
			int result = command_parse(&command_buffer, &train_id, &train_speed, &cmd);
			if (result != -1) {
				// user entered a valid command, sends command to train and updates user interface
              	command_handle(&cmd);
			}
			// clears command_buffer
			command_clear(&command_buffer);
		}
		else {
			command_buffer.data[command_buffer.pos++] = c;
			Putc(COM2, c);
		}
	}
	Exit();
}
