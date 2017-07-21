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
} Sensor;

void sensor_initialization();
int sensor_to_num(Sensor sensor);
Sensor num_to_sensor(int num);

typedef enum {
	MIN_SPEED = 0,
	MAX_SPEED = 14,
	REVERSE = 15,
	START = 96,
	HALT = 97
} Train_state;

/* Calibration */
typedef struct Calibration_package {
    int src;
    int dest;
    int distance;
    int real_velocity;  // in um / tick
    int velocity;       // virtual velocity measured in um / tick
} Calibration_package;

/* Velocity */
#define ALPHA           0.9
#define MAX_VELOCITY    8000    // um / tick
typedef struct Velocity_model {
    int train_id;
    int stopping_distance[MAX_SPEED + 1];   // in mm
    double velocity[MAX_SPEED + 1];         // in um / tick
} Velocity_model;


typedef struct Train_br_switch{
    int sensor_stop;
    char id;
    char state;
} Train_br_switch;

// switches to flip such that train can at a sensor 
// sensors that locate right before the br switch
#define BR_LIFO_SIZE    10
typedef struct Br_lifo{
    Train_br_switch br_lifo[BR_LIFO_SIZE];
    int br_lifo_top;
} Br_lifo;

/* Train */
typedef struct Train {
	char id;
	int speed;
    int last_stop;
    int predict_stop;
    int last_sensor_triggered_time;
    int deaccelarate_stop;
    int park_delay_time;
    Br_lifo br_lifo_struct;
    Velocity_model velocity_model;
} Train;

int track_node_name_to_num(char *name);
void velocity69_initialization(Velocity_model *velocity_model); 
void velocity71_initialization(Velocity_model *velocity_model); 
void velocity58_initialization(Velocity_model *velocity_model); 
void velocity_update(int speed, double real_velocity, Velocity_model *velocity_model);

/* Short walk */
typedef struct Walk_table {
	int walk_data[3][3][9];
} Walk_table;
int train_id_to_idx(int train_id);
int speed_to_idx(int speed);
int distance_to_idx(int distance);
void walk_table_initialization(Walk_table *walk_table);
int walk_table_lookup(Walk_table *walk_table, int train_id, int speed, int distance);

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
    MAP,		/* declare the current using track is A or B */
    KC,			/* used to collect shortwalks data */
	WALK		/* Short walks for 10 - 90cm */
} Train_cmd_type;
typedef struct {
	Train_cmd_type type;
	char arg0;
	char arg1;
    char arg2;
} Command;
typedef struct Command_buffer
{
	char data[COMMAND_SIZE];
	int pos;
} Command_buffer;

Command get_tr_command(char id, char speed);
Command get_sw_command(char id, char state);
Command get_sensor_command();
Command get_tr_stop_command(char id);
Command get_br_command(char group, char id);
Command get_kc_command(char speed, char delay_time);
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
