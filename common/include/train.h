#ifndef __TRAIN_H__
#define __TRAIN_H__
#include <define.h>
#include <fifo.h>
#include <clock.h>

/* Train */
#define TRAINS 80
typedef struct Train {
	char id;
	int speed;
} Train;
typedef enum {
	MIN_SPEED = 0,
	MAX_SPEED = 14,
	REVERSE = 15,
	START = 96,
	HALT = 97
} Train_state;

/* Switches */
#define NUM_SWITCHES 22
typedef struct Switch {
	char id;
	char state;
} Switch;
typedef enum {
	SOLENOID_OFF = 32,
	STRAIGHT = 33, 
	CURVE = 34
} SWITCH_STATE;

/*
 * Initialize all switches except switch 19 and 21 to curved
 */
void initialize_switch();

/*
 * Test switches initialization buying set switches the opposite state
*/
void test_initialize_switch();

/*
 * Convert a switch id to a byte to be sent to the train controller
 */
char switch_id_to_byte(uint8 id);

/*
 * Convert a switch state to a byte to be sent to the train controller
 */
char switch_state_to_byte(char state);

/* Sensors */
#define SENSOR_GROUPS 5
#define SENSORS_PER_GROUP 16
#define SENSOR_QUERY 128 + SENSOR_GROUPS
typedef struct Sensor {
	char group;
	char id;
	int triggered_time;
	int triggered_poll;
} Sensor;
void sensor_initialization();
int sensor_to_num(Sensor sensor);
Sensor num_to_sensor(int num);

/* Train */
#define SENSOR_LIFO_SIZE 1000
typedef struct Train_server {
	int sensor_reader_tid;
	int is_shutdown;
	fifo_t cmd_fifo;
	Sensor sensor_lifo[SENSOR_LIFO_SIZE];
	int sensor_lifo_top;
    int switches_status[NUM_SWITCHES];
} Train_server;

/* Train commands */
#define COMMAND_SIZE 100
typedef enum {
	TR, 		/* Set any train in motion at the desired speed */
	RV, 		/* The train should reverse direction. */
	SW, 		/* Throw the given switch to straight (S) or curved (C). */
	GO, 		/* Start the train controller */
	STOP,		/* Stop the train controller */
    BR,         /* Flip the switches to get the train from one point to another */
	SENSOR 		/* Dump sensor modules */
} Train_cmd_type;
typedef struct {
	Train_cmd_type type;
	char arg0;
	char arg1;
} Command;
typedef struct Command_buffer
{
	char data[COMMAND_SIZE];
	int pos;
} Command_buffer;

/* Calibration */
typedef struct Calibration_package {
	int src;
	int dest;
	int distance;
	int time;
	int velocity;
} Calibration_package;

/* Cli */
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
	Clock clock_update;
	Calibration_package calibration_update;
    Switch br_update[10];
} Cli_request;
typedef struct Cli_server {
	fifo_t cmd_fifo;
	fifo_t status_update_fifo;
	int cli_io_tid;
	int cli_clock_tid;
	int is_shutdown;
} Cli_server;

/*
 * Clear the command_buffer by fill it with space
 */
void command_clear(Command_buffer *command_buffer);

/*
 * Parses the command in the command_buffer.
 * Updates ptrain_id and ptrain_speed if command_buffer stores a valid TR command.
 * Returns 0 if the command is a valid train command, and the command type and arguments are stored in pcmd.
 * Returns 1 if the command is 'q'.
 * Returns -1 otherwise.
 */
int command_parse(Command_buffer *command_buffer, Train *ptrain, Command *pcmd);

/*
 * Based on pcmd, send bytes to train.
 * Excluding SENSOR and SHUTDOWN 
 */
void command_handle(Command *pcmd, Train_server *train_server);

#endif // __TRAIN_H__
