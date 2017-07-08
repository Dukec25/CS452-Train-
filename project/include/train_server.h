#ifndef __TRAIN_SERVER__
#define __TRAIN_SERVER__
#include <train.h>
#include <fifo.h>
#include <cli_server.h>

typedef enum {
	TS_NULL,
	TS_WANT_CLI_REQ,
	TS_COMMAND
} TS_request_type;

typedef struct TS_request {
	TS_request_type type;
	Command cmd;
} TS_request;

typedef struct Train_server {
	int is_shutdown;

	int is_special_cmd;
	Command special_cmd;

#define COMMAND_FIFO_SIZE	100
	Command cmd_fifo[COMMAND_FIFO_SIZE];
	int cmd_fifo_head;
	int cmd_fifo_tail;

#define CLI_REQ_FIFO_SIZE	100
	Cli_request cli_req_fifo[CLI_REQ_FIFO_SIZE];
	int cli_req_fifo_head;
	int cli_req_fifo_tail;

	Train train;

#define SENSOR_LIFO_SIZE	100
	Sensor sensor_lifo[SENSOR_LIFO_SIZE];
	int sensor_lifo_top;
	int last_stop;	// last sensor converted to num
	int num_sensor_query;

    int switches_status[NUM_SWITCHES];

    Switch br_update[10];			// switches to flip such that train can at a sensor 
    track_node track[TRACK_MAX];    // Data for the current using track
	Velocity_data velocity14_data;
	Velocity_data velocity10_data;
	Velocity_data velocity8_data;
	Velocity_data velocity6_data;

    Velocity_data *current_velocity_data;
    Map cli_map;
} Train_server;

void train_server_init(Train_server *train_server);
void train_server();
void sensor_reader_task();
void sensor_handle(Train_server *train_server);
void dc_handle(Train_server *train_server, Command dc_cmd);
void br_handle(Train_server *train_server, Command br_cmd);
void park_handle(Train_server *train_server, Command park_cmd);

/* helper functions */
Sensor parse_stop_sensor(Command cmd);
void push_cmd_fifo(Train_server *train_server, Command cmd);
void pop_cmd_fifo(Train_server *train_server, Command *cmd);
void push_cli_req_fifo(Train_server *train_server, Cli_request cli_req);
void pop_cli_req_fifo(Train_server *train_server, Cli_request *cli_req);
void push_sensor_lifo(Train_server *train_server, Sensor sensor);
void pop_sensor_lifo(Train_server *train_server, Sensor *sensor);

#endif // __TRAIN_SERVER__
