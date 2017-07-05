#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <clock.h>
#include <string.h>
#include <bwio.h>
#include <irq_io.h>

Cli_request get_train_command_request(Command cmd)
{
	Cli_request train_cmd_request;
	train_cmd_request.type = CLI_TRAIN_COMMAND;
	train_cmd_request.cmd = cmd;
	return train_cmd_request;	
}

Cli_request get_update_train_request(char id, char speed)
{
	Cli_request update_train_request;
	update_train_request.type = CLI_UPDATE_TRAIN;
	update_train_request.train_update.id = id; 
	update_train_request.train_update.speed = speed;
	return update_train_request;
}

Cli_request get_update_switch_request(char id, char state)
{
	Cli_request update_switch_request;
	update_switch_request.type = CLI_UPDATE_SWITCH;
	update_switch_request.switch_update.id = id; 
	update_switch_request.switch_update.state = state;	
	return update_switch_request;
}

Cli_request get_update_sensor_request(Sensor sensor, int last_stop, int next_stop)
{
	Cli_request update_sensor_request;
	update_sensor_request.type = CLI_UPDATE_SENSOR;
	update_sensor_request.sensor_update = sensor;
	update_sensor_request.last_sensor_update = last_stop;
	update_sensor_request.next_sensor_update = next_stop;
	return update_sensor_request;	
}

Cli_request get_update_calibration_request(int last_stop, int current_stop, int distance, int time, int velocity)
{
	Cli_request update_calibration_request;
	update_calibration_request.type = CLI_UPDATE_CALIBRATION;
	update_calibration_request.calibration_update.src = last_stop;
	update_calibration_request.calibration_update.dest = current_stop;
	update_calibration_request.calibration_update.distance = distance;
	update_calibration_request.calibration_update.time = time;
	update_calibration_request.calibration_update.velocity = velocity;
	return update_calibration_request;
}

Cli_request get_update_clock_request(Clock clock)
{
	Cli_request update_clock_request;
	update_clock_request.type = CLI_UPDATE_CLOCK;
	update_clock_request.clock_update = clock;
	return update_clock_request;	
}

Cli_request get_shutdown_request()
{
	Cli_request shutdown_request;
	shutdown_request.type = CLI_SHUTDOWN;	
	return shutdown_request;
}

void cli_startup()
{
	int row, col;

	// clear screen
	bw_cls();

	// draw borders
	for (col = LEFT_BORDER + 1; col <= RIGHT_BORDER - 1; col++) {
		// upper border
		bw_pos(UPPER_BORDER, col);
		bwputc(COM2, '-');
		// status border
		bw_pos(STATUS_BORDER, col);
		bwputc(COM2, '-');
		// label border
		bw_pos(LABEL_BORDER, col);
		bwputc(COM2, '-');
		// bottom borders
		bw_pos(BOTTOM_BORDER, col);
		bwputc(COM2, '-');
	}
	// left, middle, and right borders
	for (row = UPPER_BORDER; row <= BOTTOM_BORDER; row++) {
		bw_pos(row, LEFT_BORDER);
		bwputc(COM2, '|');
		bw_pos(row, MIDDLE_BORDER);
		bwputc(COM2, '|');
		bw_pos(row, RIGHT_BORDER);
		bwputc(COM2, '|');
	}

	// Place clock
	bw_pos(CLOCK_ROW, CLOCK_COL);
	bwputstr(COM2, "000:00:0");

	// Place train
	bw_pos(TRAIN_ROW, TRAIN_COL);
	bwputstr(COM2, "tr 00 00");

	// Place sensors
	bw_pos(SENSOR_LABEL_ROW, SENSOR_COL);
	bwputstr(COM2, "Sensors");
	int sensor_num;
    bwprintf(COM2, "\033[32m"); // make sensor display green
	for (sensor_num = 0; sensor_num < SENSOR_GROUPS * SENSORS_PER_GROUP; sensor_num++) {
		Sensor sensor = num_to_sensor(sensor_num);
		int row = SENSOR_ROW + sensor.id * SENSOR_INDENT_HEIGHT;
		int col = SENSOR_COL + sensor.group * SENSOR_INDENT_WIDTH;
		bw_pos(row, col);
		bwprintf(COM2, "%c%s%d", SENSOR_LABEL_BASE + sensor.group, sensor.id < 10 ? "0" : "", sensor.id);
	}
    bwprintf(COM2, "\033[0m"); // reset special format

	// Place switches
	bw_pos(SWITCH_LABEL_ROW, SWITCH_COL);
	bwputstr(COM2, "Switches");
	int sw;
    bwprintf(COM2, "\033[36m"); // make switches display cyan
	for (sw = 0; sw < NUM_SWITCHES; sw++) {
		int sw_address = switch_id_to_byte(sw + 1);
		// Place sw id
		bw_pos(SWITCH_ROW + sw, SWITCH_COL);
		bwputx(COM2, sw_address);
	}
    bwprintf(COM2, "\033[0m"); // reset special format

	// Place input cursor at the end
	bw_pos(HEIGHT, 0);
	bwputstr(COM2, "> ");

	// Save screen setup
	bw_save();
}

void cli_track_startup()
{
	bw_save();

	int row, col;

	// draw borders
	for (col = TRACK_DATA_LEFT_BORDER + 1; col <= TRACK_DATA_RIGHT_BORDER - 1; col++) {
		// upper border
		bw_pos(TRACK_DATA_UPPER_BORDER, col);
		bwputc(COM2, '-');
		// status border
		bw_pos(TRACK_DATA_STATUS_BORDER, col);
		bwputc(COM2, '-');
		// bottom borders
		bw_pos(TRACK_DATA_BOTTOM_BORDER, col);
		bwputc(COM2, '-');
	}
	// left, middle, and right borders
	for (row = TRACK_DATA_UPPER_BORDER; row <= TRACK_DATA_BOTTOM_BORDER; row++) {
		bw_pos(row, TRACK_DATA_LEFT_BORDER);
		bwputc(COM2, '|');
		bw_pos(row, TRACK_DATA_RIGHT_BORDER);
		bwputc(COM2, '|');
	}

	// Place trackA
	bw_pos(TRACK_DATA_STATUS_BORDER - 1, TRACK_DATA_LABEL_COL);
	bwputstr(COM2, "Track A (mm, cm/sec)");
	bw_restore();
}

void cli_user_input(Command_buffer *command_buffer)
{
    /*irq_pos(HEIGHT, 0);*/
	command_buffer->data[command_buffer->pos] = '\0';
    /*irq_printf(COM2, "> %s", command_buffer->data);*/
    bwprintf(COM2, "\r\n");
}

void cli_update_clock(Clock clock)
{
	irq_save();
	// Place clock
	irq_pos(CLOCK_ROW, CLOCK_COL);
	irq_printf(COM2, "%s%d:%s%d:%d",
					 clock.min < 100 ? (clock.min < 10 ? "00" : "0") : "",
					 clock.min,
					 clock.sec < 10 ? "0" : "",
					 clock.sec,
					 clock.tenth_sec);
	irq_restore();
}

void cli_update_train(Train train)
{
	irq_save();
	irq_pos(TRAIN_ROW, TRAIN_COL);
	irq_printf(COM2, "tr %d %s%d", train.id, train.speed < 10 ? "0" : "", train.speed);
	irq_restore();
}

void cli_update_switch(Switch sw)
{
	irq_save();
	irq_pos(SWITCH_ROW + sw.id - 1, RIGHT_BORDER - 1);
	Putc(COM2, toupper(sw.state));
	irq_restore();
}

void cli_update_sensor(Sensor sensor, int last_sensor_update, int next_sensor_update)
{
	irq_save();

	Sensor last_sensor = num_to_sensor(last_sensor_update);
	int last_row = SENSOR_ROW + last_sensor.id * SENSOR_INDENT_HEIGHT;
	int last_col = SENSOR_COL + last_sensor.group * SENSOR_INDENT_WIDTH + SENSOR_LABEL_WIDTH;
	irq_pos(last_row, last_col);
	irq_printf(COM2, "%s", "  ");

	int row = SENSOR_ROW + sensor.id * SENSOR_INDENT_HEIGHT;
	int col = SENSOR_COL + sensor.group * SENSOR_INDENT_WIDTH + SENSOR_LABEL_WIDTH;
	irq_pos(row, col);
	irq_printf(COM2, "%s", "<-");

	Sensor next_sensor = num_to_sensor(next_sensor_update);
	irq_pos(SENSOR_PREDICTION_ROW, SENSOR_PREDICTION_COL);
	irq_printf(COM2, "Next Sensor: %c%s%d", SENSOR_LABEL_BASE + next_sensor.group, next_sensor.id < 10 ? "0" : "", next_sensor.id);

	irq_restore();
}

void cli_update_track(Calibration_package calibration_pkg, int updates)
{
	if (calibration_pkg.src == -1) {
		return;
	}
	irq_save();
	irq_printf(COM2, "\033[33m");
	Sensor src = num_to_sensor(calibration_pkg.src);
	Sensor dest = num_to_sensor(calibration_pkg.dest);
    irq_pos(updates % 22 + 1, TRACK_DATA_COL + updates / 22 % 6 * TRACK_DATA_LENGTH);
	/*irq_pos(updates % HEIGHT + 1, TRACK_DATA_COL);	*/
	irq_printf(COM2, "%c%d->%c%d,%d,%d[10ms],%d[10ms]",
				src.group + SENSOR_LABEL_BASE, src.id, dest.group + SENSOR_LABEL_BASE, dest.id,
				calibration_pkg.distance, calibration_pkg.time, calibration_pkg.velocity);
    irq_printf(COM2, "\033[0m"); // reset special format
	irq_restore();
}
