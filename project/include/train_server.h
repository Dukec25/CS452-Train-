#ifndef __TRAIN_SERVER__
#define __TRAIN_SERVER__
#include <train.h>
#include <fifo.h>
#include <cli_server.h>
#include <track_server.h>
#include <workers.h>

#define GO_CMD_FINAL_SPEED 10 
#define GO_CMD_START_SPEED 4

typedef struct Track_result{
    int park_delay_time;
    int deaccelarate_stop;
    int reverse;
    int train_id;
} Track_result;

typedef struct Delay_request{
    vint delay_time;
    int train_id;
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
    TS_TRAIN_TO_TRACK_REQ,
    TS_PARK_SERVER,
    TS_DELAY_TIME_UP
} TS_request_type;

typedef struct TS_request {
	TS_request_type type;
	Command cmd;
    Track_result track_result;
    Delay_result delay_result;
} TS_request;

typedef struct Train_server {
	int is_shutdown;
    int train_idx; // used mainly by tr command

#define TRACK_REQ_FIFO_SIZE  100
    Track_request track_req_fifo[TRACK_REQ_FIFO_SIZE];
    int track_req_fifo_head;
    int track_req_fifo_tail;

#define COMMAND_FIFO_SIZE	100
	Command cmd_fifo[COMMAND_FIFO_SIZE];
	int cmd_fifo_head;
	int cmd_fifo_tail;

#define CLI_REQ_FIFO_SIZE	100
	Cli_request cli_req_fifo[CLI_REQ_FIFO_SIZE];
	int cli_req_fifo_head;
	int cli_req_fifo_tail;

#define MAX_NUM_TRAINS 5 
	Train trains[MAX_NUM_TRAINS];

    int switches_status[NUM_SWITCHES];

// switches to flip such that train can at a sensor 
// sensors that locate right before the br switch
#define BR_LIFO_SIZE    10
    Train_br_switch br_lifo[BR_LIFO_SIZE];
    int br_lifo_top;

    track_node track[TRACK_MAX];    // Data for the current using track

    Map cli_map;
    int cli_courier_on_wait;
    int track_courier_on_wait;

    // -1 for normal state for everything, 0 for init state of go
    //  1 for one train gets initial data, 2 for ready state
    int go_cmd_state; 
} Train_server;

void train_server_init(Train_server *train_server);
void train_server();
void sensor_reader_task();
void sensor_handle(Train_server *train_server, int delay_task_tid);
void dc_handle(Train_server *train_server, Command dc_cmd);

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


#endif // __TRAIN_SERVER__
