#include <train_server.h>
#include <cli.h>

Sensor parse_stop_sensor(Command cmd)
{
	Sensor stop_sensor;
	stop_sensor.group = toupper(cmd.arg0) - SENSOR_LABEL_BASE;
	stop_sensor.id = cmd.arg1;
	return stop_sensor;
}

void push_park_req_fifo(Train_server *train_server, Park_request park_req)
{
    int park_fifo_put_next = train_server->park_req_fifo_head + 1;
    if (park_fifo_put_next != train_server->park_req_fifo_tail) {
        if (park_fifo_put_next >= PARK_REQ_FIFO_SIZE) {
            park_fifo_put_next = 0;
        }
    }
    train_server->park_req_fifo[train_server->park_req_fifo_head] = park_req;
    train_server->park_req_fifo_head = park_fifo_put_next;  
}

void pop_park_req_fifo(Train_server *train_server, Park_request *park_req)
{
    int park_fifo_get_next = train_server->park_req_fifo_tail + 1;
    if (park_fifo_get_next >= PARK_REQ_FIFO_SIZE) {
        park_fifo_get_next = 0;
    }
    *park_req = train_server->park_req_fifo[train_server->park_req_fifo_tail];
    train_server->park_req_fifo_tail = park_fifo_get_next;  
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

void push_br_lifo(Train_server *train_server, Train_br_switch br_switch)
{
    if (train_server->br_lifo_top != BR_LIFO_SIZE - 1) {
        train_server->br_lifo_top += 1;
        train_server->br_lifo[train_server->br_lifo_top] = br_switch;
    }
}

void pop_br_lifo(Train_server *train_server)
{
    /**br_switch = train_server->br_lifo[train_server->br_lifo_top];*/
    if(train_server->br_lifo_top == -1){
        // lifo is empty 
        return;
    }
    train_server->br_lifo_top -= 1;
}

int peek_br_lifo(Train_server *train_server, Train_br_switch *br_switch)
{
    if(train_server->br_lifo_top == -1){
        return -1;
    }
    *br_switch = train_server->br_lifo[train_server->br_lifo_top];
    return 0;
}


int convert_sw_track_data(int num, int type){
    int result = 80 + (num-1)*2;
    if(type){
        result += 1;
    }
    return result;
}


/*void push_sensor_lifo(Train_server *train_server, Sensor sensor)*/
/*{*/
    /*if (train_server->sensor_lifo_top != SENSOR_LIFO_SIZE - 1) {*/
        /*train_server->sensor_lifo_top += 1;*/
        /*train_server->sensor_lifo[train_server->sensor_lifo_top] = sensor;*/
    /*}*/
/*}*/

/*void pop_sensor_lifo(Train_server *train_server, Sensor *sensor)*/
/*{*/
    /**sensor = train_server->sensor_lifo[train_server->sensor_lifo_top];*/
    /*train_server->sensor_lifo_top -= 1;*/
/*}*/
