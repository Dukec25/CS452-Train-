#ifndef __CLI_H__
#define __CLI_H__

#include <fifo.h>
#include <clock.h>
#include <train.h>

#define WIDTH 44
#define HEIGHT 32
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

/*
 * Draw the initial command line interface
 */
void cli_startup();

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
void cli_update_sensor(Sensor sensor, int updates);

#endif // __CLI_H__
