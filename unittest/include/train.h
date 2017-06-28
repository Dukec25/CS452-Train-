#ifndef __TRAIN_H__
#define __TRAIN_H__
#include <define.h>
#include <track_data.h>

/* Switches */
#define NUM_SWITCHES 22
typedef struct Switch {
	char id;
	char state;
} Switch;

#define SENSOR_GROUPS 5
#define SENSORS_PER_GROUP 16
#define SENSOR_QUERY 128 + SENSOR_GROUPS
typedef struct Sensor {
	char group;
	char id;
	int triggered_time;
	int triggered_query;
} Sensor;
int sensor_to_num(Sensor sensor);
Sensor num_to_sensor(int num);

/* Velocity */
#define VELOCITY_DATA_LENGTH	80
#define MAX_NUM_VELOCITIES		3
typedef struct Velocity_node {
	int src;
	int updates;
	int num_velocity;
	int dest[MAX_NUM_VELOCITIES];
	int velocity[MAX_NUM_VELOCITIES];
} Velocity_node;
typedef struct Velocity_data {
	Velocity_node node[TRACK_MAX];	// virtual velocity measured in [tick]
	int stopping_distance;	// mm
} Velocity_data;
int track_node_name_to_num(char *name);
void velocity14_initialization(Velocity_data *velocity_data); 
int velocity_lookup(int src, int dest, Velocity_data *velocity_data);
void velocity_update(int src, int dest, int new_velocity, Velocity_data *velocity_data);

/* Train */
#define TRAINS 80
typedef struct Train {
	char id;
	int speed;
} Train;
#endif // __TRAIN_H__
