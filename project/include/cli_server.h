#ifndef __CLI_SERVER__
#define __CLI_SERVER__
#include <train.h>
#include <fifo.h>
#include <clock.h>

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
	Train train_update;
	Switch switch_update;

	Sensor sensor_update;
	int	last_sensor_update;
	int next_sensor_update;

	Clock clock_update;
	Calibration_package calibration_update;
} Cli_request;

Cli_request get_train_command_request(Command cmd);
Cli_request get_update_train_request(char id, char speed);
Cli_request get_update_switch_request(char id, char state);
Cli_request get_update_sensor_request(Sensor sensor, int last_stop, int next_stop);
Cli_request get_update_calibration_request(int last_stop, int current_stop, int distance, int time, int velocity);
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
