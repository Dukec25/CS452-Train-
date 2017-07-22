#include <train_server.h>
#include <cli.h>

Sensor parse_stop_sensor(Command cmd)
{
	Sensor stop_sensor;
	stop_sensor.group = toupper(cmd.arg0) - SENSOR_LABEL_BASE;
	stop_sensor.id = cmd.arg1;
	return stop_sensor;
}

void push_track_req_fifo(Train_server *train_server, Track_request track_req)
{
    int track_fifo_put_next = train_server->track_req_fifo_head + 1;
    if (track_fifo_put_next != train_server->track_req_fifo_tail) {
        if (track_fifo_put_next >= TRACK_REQ_FIFO_SIZE) {
            track_fifo_put_next = 0;
        }
    }
    train_server->track_req_fifo[train_server->track_req_fifo_head] = track_req;
    train_server->track_req_fifo_head = track_fifo_put_next;  
}

void pop_track_req_fifo(Train_server *train_server, Track_request *track_req)
{
    int track_fifo_get_next = train_server->track_req_fifo_tail + 1;
    if (track_fifo_get_next >= TRACK_REQ_FIFO_SIZE) {
        track_fifo_get_next = 0;
    }
    *track_req = train_server->track_req_fifo[train_server->track_req_fifo_tail];
    train_server->track_req_fifo_tail = track_fifo_get_next;  
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

void push_br_lifo(Br_lifo *br_lifo_struct, Train_br_switch br_switch)
{
    if (br_lifo_struct->br_lifo_top != BR_LIFO_SIZE - 1) {
        br_lifo_struct->br_lifo_top += 1;
        br_lifo_struct->br_lifo[br_lifo_struct->br_lifo_top] = br_switch;
    }
}

void pop_br_lifo(Br_lifo *br_lifo_struct)
{
    /**br_switch = train_server->br_lifo[train_server->br_lifo_top];*/
    if(br_lifo_struct->br_lifo_top == -1){
        // lifo is empty 
        return;
    }
    br_lifo_struct->br_lifo_top -= 1;
}

int peek_br_lifo(Br_lifo *br_lifo_struct, Train_br_switch *br_switch)
{
    if(br_lifo_struct->br_lifo_top == -1){
        return -1;
    }
    *br_switch = br_lifo_struct->br_lifo[br_lifo_struct->br_lifo_top];
    return 0;
}

void push_track_cmd_fifo(Track_cmd_fifo_struct *track_cmd_fifo_struct, Track_cmd track_cmd)
{
    int track_cmd_fifo_put_next = track_cmd_fifo_struct->track_cmd_fifo_head + 1;
    if (track_cmd_fifo_put_next != track_cmd_fifo_struct->track_cmd_fifo_tail){
        if (track_cmd_fifo_put_next >= TRACK_CMD_FIFO_SIZE){
            track_cmd_fifo_put_next = 0;
        }
    }
    track_cmd_fifo_struct->track_cmd_fifo[track_cmd_fifo_struct->track_cmd_fifo_head] = track_cmd;
    track_cmd_fifo_struct->track_cmd_fifo_head = track_cmd_fifo_put_next;
}

void pop_track_cmd_fifo(Track_cmd_fifo_struct *track_cmd_fifo_struct, Track_cmd *track_cmd)
{
    int track_cmd_fifo_get_next = track_cmd_fifo_struct->track_cmd_fifo_tail + 1;
    if (track_cmd_fifo_get_next >= TRACK_CMD_FIFO_SIZE){
        track_cmd_fifo_get_next = 0;
    }
    *track_cmd = track_cmd_fifo_struct->track_cmd_fifo[track_cmd_fifo_struct->track_cmd_fifo_tail];
    track_cmd_fifo_struct->track_cmd_fifo_tail = track_cmd_fifo_get_next;
}
