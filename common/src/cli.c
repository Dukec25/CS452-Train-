#include <bwio.h>
#include <string.h>
#include <cursor.h>
#include <cli.h>
#include <train.h>

/* horizonal borders */
#define UPPER_BORDER 		1
#define STATUS_BORDER 		UPPER_BORDER + 2
#define LABEL_BORDER 		STATUS_BORDER + 2
#define BOTTOM_BORDER 		HEIGHT - 2
/* vertical borders */
#define LEFT_BORDER 		0
#define RIGHT_BORDER 		LEFT_BORDER + WIDTH
#define MIDDLE_BORDER 		RIGHT_BORDER - RIGHT_COL_WIDTH
/* Clock */
#define CLOCK_ROW		STATUS_BORDER - 1
#define CLOCK_COL		LEFT_BORDER + 2
/* Train */
#define TRAIN_ROW		STATUS_BORDER - 1
#define TRAIN_COL		MIDDLE_BORDER + 1
/* Sensor */
#define SENSOR_LABEL_ROW	LABEL_BORDER - 1
#define SENSOR_ROW		LABEL_BORDER + 2
#define SENSOR_COL		LEFT_BORDER + 2
#define SENSORS_PER_ROW		4
#define SENSORS_PER_COL		22
#define SENSOR_LABEL_BASE	'A'
#define SENSOR_INDENT_WIDTH	8
/* Switch */
#define SWITCH_LABEL_ROW 	LABEL_BORDER - 1
#define SWITCH_ROW		LABEL_BORDER + 1
#define SWITCH_COL 		MIDDLE_BORDER + 1

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
		// Place state
		bw_pos(SWITCH_ROW + sw, RIGHT_BORDER - 1);
		bwputc(COM2, (sw == 19 || sw == 21) ? 'S' : 'C');
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
	// Place clock
	irq_pos(CLOCK_ROW, CLOCK_COL);
    irq_printf(COM2, "%s%d:%s%d:%d",
                     pclock->min < 100 ? (pclock->min < 10 ? "00" : "0") : "",
                     pclock->min,
                     pclock->sec < 10 ? "0" : "",
                     pclock->sec,
                     pclock->tenth_sec);
}

void cli_update_train(char id, char speed)
{
	// Place train
	irq_pos(TRAIN_ROW, TRAIN_COL);
	irq_printf(COM2, "tr %d %s%d", id, speed < 10 ? "0" : "", speed);
}

void cli_update_switch(char id, char state)
{
	// Place state
	irq_pos(SWITCH_ROW + id - 1, RIGHT_BORDER - 1);
	Putc(COM2, toupper(state));
}

void cli_update_sensor(char group, char id, int updates)
{
	// Save screen
	irq_save();
	// Place sensor
	irq_pos(SENSOR_ROW + updates % SENSORS_PER_COL, SENSOR_COL + updates / SENSORS_PER_COL * SENSOR_INDENT_WIDTH);
	irq_printf(COM2, "%c0%d", SENSOR_LABEL_BASE + group - 2, id);
	// Restore screen setup
	irq_restore();
}
