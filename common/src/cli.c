#include <bwio.h>
#include <string.h>
#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <user_functions.h>
#include <irq_io.h>

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

	// Place switches
	bw_pos(SWITCH_LABEL_ROW, SWITCH_COL);
	bwputstr(COM2, "Switches");
	int sw;
	for (sw = 0; sw < NUM_SWITCHES; sw++) {
		int sw_address = switch_id_to_byte(sw + 1);
		// Place sw id
		bw_pos(SWITCH_ROW + sw, SWITCH_COL);
		bwputx(COM2, sw_address);
	}

	// Place input cursor at the end
	bw_pos(HEIGHT, 0);
	bwputstr(COM2, "> ");

	// Save screen setup
	bw_save();
}

void cli_user_input(Command_buffer *command_buffer)
{
	irq_pos(HEIGHT, 0);
	command_buffer->data[command_buffer->pos] = '\0';
	irq_printf(COM2, "> %s", command_buffer->data);
}

void cli_update_clock(Clock *pclock)
{
	irq_save();
	// Place clock
	irq_pos(CLOCK_ROW, CLOCK_COL);
    irq_printf(COM2, "%s%d:%s%d:%d",
                     pclock->min < 100 ? (pclock->min < 10 ? "00" : "0") : "",
                     pclock->min,
                     pclock->sec < 10 ? "0" : "",
                     pclock->sec,
                     pclock->tenth_sec);
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

void cli_update_sensor(Sensor sensor, int updates)
{
	irq_save();
	int row = SENSOR_ROW + updates % SENSORS_PER_COL;
	int col = SENSOR_COL + (updates / SENSORS_PER_COL % SENSORS_PER_ROW) * SENSOR_INDENT_WIDTH;
	irq_pos(row, col);
	irq_printf(COM2, "%c%s%d", SENSOR_LABEL_BASE + sensor.group, sensor.id < 10 ? "0" : "", sensor.id);
	irq_restore();
}
