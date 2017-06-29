#ifndef __TRAIN_TASK__
#define __TRAIN_TASK__
#include <train.h>
#include <fifo.h>

void train_task_startup();

typedef enum Handshake {
	HANDSHAKE_AKG,
	HANDSHAKE_SHUTDOWN
} Handshake;

/* Train server */
#define SENSOR_LIFO_SIZE	100
#define COMMAND_FIFO_SIZE	100
typedef struct Train_server {
	int sensor_reader_tid;

	Command cmd_fifo[COMMAND_FIFO_SIZE];
	int cmd_fifo_head;
	int cmd_fifo_tail;

	Train train;

	Sensor sensor_lifo[SENSOR_LIFO_SIZE];
	int sensor_lifo_top;
	int last_stop;	// last sensor converted to num
	int num_sensor_query;

    int switches_status[NUM_SWITCHES];

    Switch br_update[10];			// switches to flip such that train can at a sensor 
} Train_server;
void train_server_init(Train_server *train_server);
void train_server();
void sensor_reader_task();
void stopping_distance_collector_task();
void br_task();
void park_task();

/* Cli server*/
typedef struct Cli_server {
	fifo_t cmd_fifo;
	fifo_t status_update_fifo;
	int cli_io_tid;
	int cli_clock_tid;
} Cli_server;
void cli_server();
void cli_clock_task();
void cli_io_task();
#endif // __TRAIN_TASK__
