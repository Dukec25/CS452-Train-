#ifndef __TRAIN_H__
#define __TRAIN_H__
#include <define.h>
#include <track_data.h>

/* Switches */
#define NUM_SWITCHES 22
#define NUM_SENSORS 80
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
void reverse_initialize_switch();
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
	int triggered_query;
} Sensor;
void sensor_initialization();
int sensor_to_num(Sensor sensor);
Sensor num_to_sensor(int num);

/* Calibration */
typedef struct Calibration_package {
	int src;
	int dest;
	int distance;
	int time; // actual time, [tick] = [10ms]
	int velocity; // virtual velocity measured in [tick]
} Calibration_package;

/* Velocity */
#define VELOCITY_DATA_LENGTH	80
#define MAX_NUM_VELOCITIES		3
typedef struct Velocity_node {
	int src;
	int num_velocity;
	int dest[MAX_NUM_VELOCITIES];
	int velocity[MAX_NUM_VELOCITIES];
	int updates[MAX_NUM_VELOCITIES];
} Velocity_node;
typedef struct Velocity_data {
	Velocity_node node[TRACK_MAX];	// virtual velocity measured in [tick]
	int stopping_distance;	// mm
} Velocity_data;
int track_node_name_to_num(char *name);
void velocity14_initialization(Velocity_data *velocity_data); 
void velocity10_initialization(Velocity_data *velocity_data);
void velocity8_initialization(Velocity_data *velocity_data);
int velocity_lookup(int src, int dest, Velocity_data *velocity_data);
void velocity_update(int src, int dest, int new_velocity, Velocity_data *velocity_data);

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

/* Command */
#define COMMAND_SIZE 100
typedef enum {
	TR, 		/* Set any train in motion at the desired speed */
	RV, 		/* The train should reverse direction. */
	SW, 		/* Throw the given switch to straight (S) or curved (C). */
	GO, 		/* Start the train controller */
	STOP,		/* Stop the train controller */
	SENSOR,		/* Dump sensor modules */
    BR,			/* Flip the switches to get the train from one point to another */
	DC,			/* Stop train to measure stopping distance */
    PARK,		/* Flip the switches to get the train from one point to another, and park train at a sensor */
    MAP        /* declare the current using track is A or B */
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
Command get_sw_command(char id, char state);
Command get_sensor_command();
Command get_tr_stop_command(char id);
Command get_br_command(char group, char id);
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
 * Excluding SENSOR, DC, PARK
 */
void command_handle(Command *pcmd);

#endif // __TRAIN_H__
