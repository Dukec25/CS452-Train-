#ifndef __TRAIN_SERVER__
#define __TRAIN_SERVER__
#include <fifo.h>
#include <cli_server.h>
#include <workers.h>

#define GO_CMD_FINAL_SPEED 10 
#define GO_CMD_START_SPEED 4

typedef enum {
    TRAIN_WANT_GUIDANCE, // select random destination and route 
    TRAIN_WANT_RESULT
} Track_request_type;

typedef enum {
	TS_NULL,
	TS_WANT_CLI_REQ,
	TS_COMMAND,
    TS_TRAIN_TO_TRACK_REQ,
    TS_TRACK_SERVER,
    TS_DELAY_TIME_UP
} TS_request_type;

typedef enum {
    TRACK_SLOW_WALK,
    TRACK_REVERSE,
    TRACK_PARK
} Track_cmd_type;

typedef struct Park_info{
    int delay_time;
    int deacceleration_stop;
} Park_info;

typedef struct Track_cmd{
    int type;
    Park_info park_info;
    int distance; // slow_walk distance 
} Track_cmd;

typedef struct Track_request{
    Train *train;
	Track_request_type type;
} Track_request;

#define TRACK_CMD_FIFO_SIZE 50
typedef struct Track_cmd_fifo_struct{
    Track_cmd track_cmd_fifo[TRACK_CMD_FIFO_SIZE];
    int track_cmd_fifo_head;
    int track_cmd_fifo_tail;
} Track_cmd_fifo_struct;

#define TRACK_NODE_LIFO_SIZE 50
typedef struct Track_node_lifo_struct{
    track_node *track_node_lifo[TRACK_NODE_LIFO_SIZE];
    int lifo_top;
} Track_node_lifo_struct;

typedef struct Delay_request{
    vint delay_time;
    int train_id;
} Delay_request;

typedef struct Delay_result {
    int train_id;
} Delay_result;

typedef struct Track_result{
    int train_id;
    Br_lifo br_lifo_struct;
    Track_cmd_fifo_struct cmd_fifo_struct; 
} Track_result;


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

    Br_lifo br_lifo_struct;
#define MAX_NUM_TRAINS 5 
	Train trains[MAX_NUM_TRAINS];
    int switches_status[NUM_SWITCHES];

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
void push_br_lifo(Br_lifo *br_lifo_struct, Train_br_switch br_switch);
void pop_br_lifo(Br_lifo *br_lifo_struct);
int peek_br_lifo(Br_lifo *br_lifo_struct, Train_br_switch *br_switch);

void push_track_cmd_fifo(Track_cmd_fifo_struct *track_cmd_fifo_struct, Track_cmd track_cmd);
void pop_track_cmd_fifo(Track_cmd_fifo_struct *track_cmd_fifo_struct, Track_cmd *track_cmd);

#endif // __TRAIN_SERVER__
