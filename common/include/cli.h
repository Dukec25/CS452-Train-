#ifndef __CLI_H__
#define __CLI_H__

#include <clock.h>
#include <train.h>

typedef enum {
	CLI_TRAIN_COMMAND,
	CLI_UPDATE_TRAIN,
	CLI_UPDATE_SENSOR,
	CLI_UPDATE_SWITCH,
	CLI_UPDATE_CLOCK,
	CLI_UPDATE_CALIBRATION,
	CLI_SHUTDOWN
} Cli_request_type;
typedef struct Cli_request {
	Cli_request_type type;
	Command cmd;
	Train train_update;
	Switch switch_update;

	Sensor sensor_update;
	int	last_sensor_update;
	int next_sensor_update;

	Clock clock_update;
	Calibration_package calibration_update;
} Cli_request;
Cli_request get_train_command_request(Command cmd);
Cli_request get_update_train_request(char id, char speed);
Cli_request get_update_switch_request(char id, char state);
Cli_request get_update_sensor_request(Sensor sensor, int last_stop, int next_stop);
Cli_request get_update_calibration_request(int last_stop, int current_stop, int distance, int time, int velocity);
Cli_request get_update_clock_request(Clock clock);
Cli_request get_shutdown_request();

#define WIDTH 46
#define HEIGHT 30
#define RIGHT_COL_WIDTH 9
#define SENSOR_LABEL_BASE 'A'

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
#define SENSOR_LABEL_ROW		LABEL_BORDER - 1
#define SENSOR_ROW				LABEL_BORDER + 2
#define SENSOR_COL				LEFT_BORDER + 2
#define SENSOR_LABEL_BASE		'A'
#define SENSOR_LABEL_WIDTH		3
#define SENSOR_INDENT_WIDTH 	7
#define SENSOR_INDENT_HEIGHT	1
#define SENSOR_PREDICTION_ROW	BOTTOM_BORDER - 2
#define SENSOR_PREDICTION_COL	SENSOR_COL
/* Switch */
#define SWITCH_LABEL_ROW 	LABEL_BORDER - 1
#define SWITCH_ROW			LABEL_BORDER + 1
#define SWITCH_COL 			MIDDLE_BORDER + 1
/* Track */
#define TRACK_DATA_WIDTH 			100
#define TRACK_DATA_LEFT_BORDER 		RIGHT_BORDER
#define TRACK_DATA_RIGHT_BORDER 	RIGHT_BORDER + TRACK_DATA_WIDTH
#define TRACK_DATA_UPPER_BORDER 	UPPER_BORDER
#define TRACK_DATA_STATUS_BORDER	TRACK_DATA_UPPER_BORDER + 2
#define TRACK_DATA_LABEL_BORDER 	TRACK_DATA_STATUS_BORDER + 2
#define TRACK_DATA_BOTTOM_BORDER 	BOTTOM_BORDER
#define TRACK_DATA_LABEL_ROW		TRACK_DATA_LABEL_BORDER - 1
#define TRACK_DATA_LABEL_COL		TRACK_DATA_LEFT_BORDER + 2
#define TRACK_DATA_ROW				TRACK_DATA_LABEL_ROW
#define TRACK_DATA_COL				TRACK_DATA_LABEL_COL
#define TRACK_DATA_PER_ROW			5
#define TRACK_DATA_PER_COL			26
#define TRACK_LABEL_LENGTH			7
#define TRACK_DATA_LENGTH			20

/*
 * Draw the initial command line interface
 */
void cli_startup();

/*
 * Draw the initial track A display
 */
void cli_track_startup();

/*
 * Display user input
 */
void cli_user_input(Command_buffer *command_buffer);

/*
 * Update the digital clock in the command line interface
 */
void cli_update_clock(Clock clock);

/*
 * Update the train status in the command line interface
 */
void cli_update_train(Train train);

/*
 * Update the switch status in the command line interface
 */
void cli_update_switch(Switch sw);

/*
 * Update the last triggered sensor in the command line interface 
 */
void cli_update_sensor(Sensor sensor, int last_sensor_update, int next_sensor_update);

/*
 * Update the track A display
 */
void cli_update_track(Calibration_package calibration_pkg, int updates);
#endif // __CLI_H__
