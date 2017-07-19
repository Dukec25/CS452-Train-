#ifndef __TRAIN_TASK__
#define __TRAIN_TASK__
#include <train_server.h>
#include <cli_server.h>
#include <track_server.h>
#include <train.h>
#include <fifo.h>

typedef enum Handshake {
	HANDSHAKE_AKG,
	HANDSHAKE_SHUTDOWN
} Handshake;

void train_task_admin();
void idle_task();

typedef enum Courier_message_type {
	COURIER_NIL,
	COURIER_WANT_CMD,
	COURIER_WANT_CLI_REQUEST,
	COURIER_HAS_CMD,
	COURIER_HAS_CLI_REQUEST
} Courier_message_type;
typedef struct Courier_message {
	Courier_message_type type;
	Command cmd;
	Cli_request cli_req;
} Courier_message;

/* Courier */
void train_command_courier();
void cli_request_courier();
void train_to_track_courier();
void track_to_train_courier();

/* Test */
void milestone1_test();

#endif // __TRAIN_TASK__
