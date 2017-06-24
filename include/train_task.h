#ifndef __TRAIN_TASK__
#define __TRAIN_TASK__
#include <define.h>
#include <bwio.h>
#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <clock_server.h>
#include <user_functions.h>
#include <debug.h>
#include <kernel.h>
#include <name_server.h>
#include <calculation.h>
#include <fifo.h>

void train_task_startup();
void clock_task();
void sensor_initialization();
void sensor_task();
void train_task();

typedef enum Handshake {
	HANDSHAKE_AKG,
	HANDSHAKE_SHUTDOWN
} Handshake;


void train_server();
void sensor_reader_task();

/* Cli */
typedef enum {
	CLI_TRAIN_COMMAND,
	CLI_UPDATE_TRAIN,
	CLI_UPDATE_SENSOR,
	CLI_UPDATE_SWITCH,
	CLI_UPDATE_CLOCK,
	CLI_SHUTDOWN
} Cli_request_type;
typedef struct Cli_request {
	Cli_request_type type;
	Command cmd;
	Train train_update;
	Switch switch_update;
	Sensor sensor_update;
	Clock clock_update;
} Cli_request;
typedef struct Cli_server {
	fifo_t cmd_fifo;
	fifo_t status_update_fifo;
	int cli_io_tid;
	int cli_clock_tid;
	int is_shutdown;
} Cli_server;

void cli_server();
void cli_clock_task();
void cli_io_task();
#endif // __TRAIN_TASK__
