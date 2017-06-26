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


void cli_server();
void cli_clock_task();
void cli_io_task();
#endif // __TRAIN_TASK__
