#ifndef __CLI_H__
#define __CLI_H__

#include <clock.h>
#include <train.h>

#define WIDTH 70 
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

/* Track map */
#define MAP_FIRST_ROW       SENSOR_ROW

/* Switch */
#define SWITCH_LABEL_ROW 	STATUS_BORDER - 1
#define SWITCH_ROW			STATUS_BORDER + 1
#define SWITCH_COL 			MIDDLE_BORDER + 1

/* Track */
#define TRACK_DATA_ROW              UPPER_BORDER
#define TRACK_DATA_COL              RIGHT_BORDER + 10
#define TRACK_DATA_PER_ROW			4
#define TRACK_DATA_PER_COL			26
#define TRACK_LABEL_LENGTH			7
#define TRACK_DATA_LENGTH			45

/* Train 69 Status */
/* Train 71 Status */
#define TRAIN_LEFT_BAR				RIGHT_BORDER + 2
#define TRAIN_DATA_ROW				STATUS_BORDER - 1
#define TRAIN_DATA_COL				TRAIN_LEFT_BAR + 35
#define TRAIN_DATA_WIDTH			30

typedef struct loc{
    int row;
    int col;
} loc;

typedef struct Map{
    char *ascii;
    loc sensors[NUM_SENSORS];
    loc switches[NUM_SWITCHES+1]; // start with 1 
    int test;
} Map;

/*
 * Draw the initial command line interface
 */
void cli_startup();

/*
 * Update the digital clock in the command line interface
 */
void cli_update_clock(Clock clock);

/*
 * Update the train status in the command line interface
 */
void cli_update_train(Train *train);

/*
 * Update the switch status in the command line interface
 */
//void cli_update_switch(Switch sw);
void cli_update_switch(Switch sw, Map *map);

/*
 * Update the last triggered sensor in the command line interface 
 */
//void cli_update_sensor(Sensor sensor, int last_sensor_update, int next_sensor_update);
void cli_update_sensor(Sensor sensor, int time, int last_sensor_update, int attributed,
						Train *train, int real_velocity, int expected_velocity, Map *map);

/*
 * Update the track
 */
void cli_update_track(Calibration_package calibration_pkg, int updates);
#endif // __CLI_H__

