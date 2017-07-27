#ifndef __CLI_SERVER__
#define __CLI_SERVER__
#include <train.h>
#include <fifo.h>
#include <clock.h>
#include <cli.h>

typedef enum {
	CLI_NULL,
	CLI_WANT_COMMAND,
	CLI_TRAIN_COMMAND,
	CLI_UPDATE_TRAIN,
	CLI_UPDATE_SENSOR,
	CLI_UPDATE_SWITCH,
	CLI_UPDATE_CLOCK,
	CLI_UPDATE_CALIBRATION,
	CLI_SHUTDOWN
} Cli_request_type;

typedef struct Cli_request {
	Cli_request_type type;

	Command cmd;

	Switch switch_update;

	Train *train_update;

	Sensor sensor_update;
	int sensor_triggered_time;
	int	last_sensor_update;
	int attributed;
	int real_velocity;
	int expected_velocity;

	Clock clock_update;

	Calibration_package calibration_update;
} Cli_request;

Cli_request get_train_command_request(Command cmd);
Cli_request get_update_train_request(Train *train_update);
Cli_request get_update_switch_request(char id, char state);
Cli_request get_update_sensor_request(Sensor sensor, int sensor_triggered_time, int last_stop,
									  int attributed, Train *train_update, int real_velocity, int expected_velocity);
Cli_request get_update_calibration_request(int last_stop, int current_stop, int distance, int real_velocity, int velocity);
Cli_request get_update_clock_request(Clock clock);
Cli_request get_shutdown_request();

typedef struct Cli_server {
	int is_shutdown;

	fifo_t cmd_fifo;
	fifo_t status_update_fifo;
} Cli_server;

void cli_server();
void cli_clock_task();
void cli_io_task();

#endif // __CLI_SERVER__
