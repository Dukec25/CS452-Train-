#include <train_task.h>

void train_task_startup()
{
	cli_startup();
    irq_io_tasks_cluster();

    Putc(COM1, START); // switches won't work without start command
    initialize_switch();
    debug(DEBUG_K4, "%s", "initialized switches");

    int tid;
	tid  = Create(PRIOR_LOW, clock_task);
    debug(DEBUG_K4, "created clock_task taskId = %d", tid);

	/*tid = Create(PRIOR_LOW, train_task);*/
       /*debug(DEBUG_K4, "created train_task taskId = %d", tid);*/

    sensor_initialization();
    tid = Create(PRIOR_LOW, sensor_task);
    debug(DEBUG_K4, "created sensor_task taskId = %d", tid);
	
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
    vint stop_sensor=16;
    vint last_stop;

    track_node tracka[TRACK_MAX];
    init_tracka(tracka);

	while (1) {
		Delay(20);	// delay 0.2 second
		Putc(COM1, SENSOR_QUERY);
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
				if (sensor_data[group] & (0x1 << id)) {
                    int actual_id; 
                    if( id + 1 <= 8){
                        actual_id = 8 - id;
                    } else{
                        actual_id = 8 + 16 - id;
                    }
                    //sensor_data actually looks like 
                    // 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
					cli_update_sensor(group, actual_id, updates++);
                    if(group*16-1 + actual_id  == stop_sensor){
                        Putc(COM1, 0); 	 	// stop	
                        Putc(COM1, 69); 	// train
                    }
                    if(last_stop != -1){
                        /*int b = cal_distance(tracka, last_stop, group*16-1 + actual_id);*/
                        /*Putc(COM2, b);*/
                        last_stop = group*16-1 + actual_id ;
                    }
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

	while(1) {
		// user I/O and send command to train
		char c = Getc(COM2);
		if (c == 'q') {
			// user hits 'q', exit
			debug(SUBMISSION, "%s", "user hits q, exiting");	
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
              	command_handle(&cmd);
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
