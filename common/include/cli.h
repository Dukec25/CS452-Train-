#ifndef __CLI_H__
#define __CLI_H__

#include <fifo.h>
#include <clock.h>
#include <train.h>

#define WIDTH 44
#define HEIGHT 32
#define RIGHT_COL_WIDTH 9
#define SENSOR_LABEL_BASE 'A'

/*
 * Draw the initial command line interface
 */
void cli_startup();

/*
 * Update the digital clock in the command line interface
 */
void cli_update_clock(Clock *pclock);

/*
 * Update the train status in the command line interface
 */
void cli_update_train(char id, char speed);

/*
 * Update the switch status in the command line interface
 */
void cli_update_switch(char id, char state);

/*
 * Update the last triggered sensor in the command line interface 
 */
void cli_update_sensor(char group, char id);

#endif // __CLI_H__
