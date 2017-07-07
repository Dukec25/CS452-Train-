#include <train_server.h>
#include <cli.h>

Sensor parse_stop_sensor(Command cmd)
{
	Sensor stop_sensor;
	stop_sensor.group = toupper(cmd.arg0) - SENSOR_LABEL_BASE;
	stop_sensor.id = cmd.arg1;
	return stop_sensor;
}

void push_cmd_fifo(Train_server *train_server, Command cmd)
{
	int cmd_fifo_put_next = train_server->cmd_fifo_head + 1;
	if (cmd_fifo_put_next != train_server->cmd_fifo_tail) {
		if (cmd_fifo_put_next >= COMMAND_FIFO_SIZE) {
			cmd_fifo_put_next = 0;
		}
	}
	train_server->cmd_fifo[train_server->cmd_fifo_head] = cmd;
	train_server->cmd_fifo_head = cmd_fifo_put_next;	
}

void pop_cmd_fifo(Train_server *train_server, Command *cmd)
{
	int cmd_fifo_get_next = train_server->cmd_fifo_tail + 1;
	if (cmd_fifo_get_next >= COMMAND_FIFO_SIZE) {
		cmd_fifo_get_next = 0;
	}
	*cmd = train_server->cmd_fifo[train_server->cmd_fifo_tail];
	train_server->cmd_fifo_tail = cmd_fifo_get_next;	
}

void push_cli_req_fifo(Train_server *train_server, Cli_request cli_req)
{
	int cli_req_fifo_put_next = train_server->cli_req_fifo_head + 1;
	if (cli_req_fifo_put_next != train_server->cli_req_fifo_tail) {
		if (cli_req_fifo_put_next >= CLI_REQ_FIFO_SIZE) {
			cli_req_fifo_put_next = 0;
		}
	}
	train_server->cli_req_fifo[train_server->cli_req_fifo_head] = cli_req;
	train_server->cli_req_fifo_head = cli_req_fifo_put_next;	
}

void pop_cli_req_fifo(Train_server *train_server, Cli_request *cli_req)
{
	int cli_req_fifo_get_next = train_server->cli_req_fifo_tail + 1;
	if (cli_req_fifo_get_next >= CLI_REQ_FIFO_SIZE) {
		cli_req_fifo_get_next = 0;
	}
	*cli_req = train_server->cli_req_fifo[train_server->cli_req_fifo_tail];
	train_server->cli_req_fifo_tail = cli_req_fifo_get_next;
}
