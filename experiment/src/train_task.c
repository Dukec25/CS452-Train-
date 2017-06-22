#include <train_task.h>

void train_task_startup()
{
	cli_startup();
    irq_io_tasks_cluster();

    Putc(COM1, START); // switches won't work without start command
    initialize_switch();
    test_initialize_switch();
    sensor_initialization();
    debug(DEBUG_K4, "%s", "initialized switches");

    int tid;
	tid  = Create(PRIOR_LOW, clock_task);
    debug(DEBUG_K4, "created clock_task taskId = %d", tid);

    tid = Create(PRIOR_LOW, sensor_task);
    debug(DEBUG_K4, "created sensor_task taskId = %d", tid);

    tid = Create(PRIOR_LOW, train_task);
    debug(DEBUG_K4, "created train_task taskId = %d", tid);
	
	Exit();
}

void clock_task()
{
	vint elapsed_tenth_sec = 0;
	// digital clock
	Clock clock;
	clock_init(&clock);

	while (1) {
		Delay(10);	// update every 100ms
		elapsed_tenth_sec++;
		//debug(DEBUG_K4, "!!!!!!delayed time interval, elapsed_tenth_sec = %d", elapsed_tenth_sec);
        clock_update(&clock, elapsed_tenth_sec);
        cli_update_clock(&clock);
	}
	Exit();
}

void sensor_initialization(){
    Delay(20);	// delay 0.2 second
    // clear up any unread sensor data
    Putc(COM1, SENSOR_QUERY);
    int group = 0;
    for (group = 0; group < SENSOR_GROUPS; group++) {
        char lower = Getc(COM1);
        char upper = Getc(COM1);
    } 
}

void sensor_task() {

	int updates = 0;

	vint register_result = RegisterAs("SENSOR_TASK");
    vint requester;
    Calibration_package cali_pkg;
    Delivery reply_msg;
    Receive(&requester, &cali_pkg, sizeof(cali_pkg));
    Reply(requester, &reply_msg, sizeof(reply_msg));

    track_node tracka[TRACK_MAX];
    init_tracka(tracka);

	while (1) {
        bwprintf(COM2, "%s", "running  ");
        /*debug(SUBMISSION, "%s", "sensor is printing");*/
        /*irq_printf(COM2, "%s", "sensor is printing");*/
        Delay(20);	// delay 0.2 second
		Putc(COM1, SENSOR_QUERY);
        /*bwprintf(COM2, "%s", "after SENSOR_QUERY\r\n");*/
		int sensor_data[SENSOR_GROUPS];
		int group = 0;
		for (group = 0; group < SENSOR_GROUPS; group++) {
			char lower = Getc(COM1);
			char upper = Getc(COM1);
			sensor_data[group] = upper << 8 | lower;
		}

		for (group = 0; group < SENSOR_GROUPS; group++) {
			int id = 0;
			for (id = 0; id < SENSORS_PER_GROUP; id++) {
                /*bwprintf(COM2, "%s", "Sensor group  ");*/
                    int actual_id; 
                    if( id + 1 <= 8){
                        actual_id = 8 - id;
                    } else{
                        actual_id = 8 + 16 - id;
                    }
                    //sensor_data actually looks like 
                    // 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
                    cli_update_sensor(group, actual_id, updates++);
                    /*irq_printf(COM2, "%s", "sensor is printing");*/
                    
                    if(group*16-1 + actual_id  == *(cali_pkg.stop_sensor)){
                        Putc(COM1, 0); 	 	// stop	
                        Putc(COM1, 69); 	// train
                    }
                    /*if(last_stop != -1){*/
                        /*[>int b = cal_distance(tracka, last_stop, group*16-1 + actual_id);<]*/
                        /*[>Putc(COM2, b);<]*/
                        /*last_stop = group*16-1 + actual_id ;*/
                    /*}*/
				}
			}
		}
	}
	Exit();
}

void train_task() {
	// command
	Command_buffer command_buffer;
	command_buffer.pos = 0;
	char train_id = 0;
	char train_speed = 0;

    vint stop_sensor = -1;
    vint last_stop = -1;

    Calibration_package calibration_package;
    calibration_package.stop_sensor = &stop_sensor;
    calibration_package.last_stop = &last_stop;

    int sensor_task_tid = WhoIs("SENSOR_TASK");
    Delivery reply_msg;
    Send(sensor_task_tid, &calibration_package, sizeof(Calibration_package), &reply_msg, sizeof(reply_msg) );

	while(1) {
		// user I/O and send command to train
		char c = Getc(COM2);
		if (c == 'q') {
			// user hits 'q', exit
			bwprintf(COM2, "user hits q, terminate the program");	
            Terminate();
			break;
		}
		else if (c == '\r') {
			// user hits ENTER
			// parse command
			Command cmd;
			int result = command_parse(&command_buffer, &train_id, &train_speed, &cmd);
			if (result != -1) {
				// user entered a valid command, sends command to train and updates user interface
                // command_handle is within common/src/train.c
              	command_handle(&cmd, &calibration_package);
			}
			// clears command_buffer
			command_clear(&command_buffer);
		}
		else {
			command_buffer.data[command_buffer.pos++] = c;
			Putc(COM2, c);
		}
	}
	Exit();
}
