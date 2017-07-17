#ifndef __TRAIN_SERVER__
#define __TRAIN_SERVER__
#include <train.h>
#include <fifo.h>
#include <cli_server.h>
#include <park_server.h>
#include <workers.h>


typedef struct Park_result{
    int park_delay_time;
    int deaccelarate_stop;
} Park_result;

// will expand in the future
typedef struct Park_request{
    //int train_id;
    Command park_cmd;
} Park_request;

typedef struct Delay_request{
    vint delay_time;
} Delay_request;

typedef struct Train_br_switch{
    int sensor_stop;
    char id;
    char state;
} Train_br_switch;

typedef enum {
	TS_NULL,
	TS_WANT_CLI_REQ,
	TS_COMMAND,
    TS_TRAIN_TO_PARK_REQ,
    TS_PARK_SERVER,
    TS_DELAY_TIME_UP
} TS_request_type;

typedef struct TS_request {
	TS_request_type type;
	Command cmd;
    Park_result park_result;
} TS_request;

typedef struct Train_server {
	int is_shutdown;

	int is_special_cmd;
	Command special_cmd;

#define PARK_REQ_FIFO_SIZE  100
    Park_request park_req_fifo[PARK_REQ_FIFO_SIZE];
    int park_req_fifo_head;
    int park_req_fifo_tail;

#define COMMAND_FIFO_SIZE	100
	Command cmd_fifo[COMMAND_FIFO_SIZE];
	int cmd_fifo_head;
	int cmd_fifo_tail;

#define CLI_REQ_FIFO_SIZE	100
	Cli_request cli_req_fifo[CLI_REQ_FIFO_SIZE];
	int cli_req_fifo_head;
	int cli_req_fifo_tail;

	Train train;

//#define SENSOR_LIFO_SIZE	100
	//Sensor sensor_lifo[SENSOR_LIFO_SIZE];
    int sensor_lifo_top;
	int last_stop;	// last sensor converted to num
    int last_sensor_triggered_time;

    int switches_status[NUM_SWITCHES];

// switches to flip such that train can at a sensor 
// sensors that locate right before the br switch
#define BR_LIFO_SIZE    10
    Train_br_switch br_lifo[BR_LIFO_SIZE];
    int br_lifo_top;

    track_node track[TRACK_MAX];    // Data for the current using track

    int deaccelarate_stop;
    int park_delay_time;

    Map cli_map;
    int cli_courier_on_wait;
    int park_courier_on_wait;
    Velocity_model velocity69_model;
    Velocity_model velocity71_model;
} Train_server;

void train_server_init(Train_server *train_server);
void train_server();
void sensor_reader_task();
void sensor_handle(Train_server *train_server, int delay_task_tid);
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
void push_br_lifo(Train_server *train_server, Train_br_switch br_switch);
void pop_br_lifo(Train_server *train_server, Train_br_switch *br_switch);
int peek_br_lifo(Train_server *train_server, Train_br_switch *br_switch);

// type = 0 branch, 1 merge
int convert_sw_track_data(int num, int type);

#endif // __TRAIN_SERVER__
