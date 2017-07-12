#include <sensor_server.h>

void sensor_server()
{
	Handshake handshake = HANDSHAKE_AKG;
	int train_server_tid = INVALID_TID;
	vint train_server_address;
	Receive(&train_server_tid, &train_server_address, sizeof(train_server_address));
	Reply(train_server_tid, &handshake, sizeof(handshake));
	Train_server *train_server = (Train_server *) train_server_address;
    
	while (train_server->is_shutdown == 0) {
        // size = num of trains 
        Sensor sensors_output[6];
        Sensor_result work_result;

		TS_request ts_request;
		ts_request.type = TS_SENSOR_SERVER;

        int num_sensor = sensor_handle(sensors_output);
        work_result.num_sensor = num_sensor;
        work_result.sensors = sensors_output;
        ts_request.sensor = work_result;
        Send(train_server_tid, &ts_request, sizeof(ts_request), &handshake, sizeof(handshake));
	}

	Handshake exit_handshake = HANDSHAKE_SHUTDOWN;
	Handshake exit_reply;
	Send(train_server_tid, &exit_handshake, sizeof(exit_handshake), &exit_reply, sizeof(exit_reply)); 
	
	Exit();
}

int sensor_handle(Sensor *sensor_output)
{
	Putc(COM1, SENSOR_QUERY);
	uint16 sensor_data[SENSOR_GROUPS];
    int num_hit_sensor = 0;
	int sensor_group = 0;
	for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
		char lower = Getc(COM1);
		char upper = Getc(COM1);
		sensor_data[(int) sensor_group] = upper << 8 | lower;
	}

	// parse sensor data
	for (sensor_group = 0; sensor_group < SENSOR_GROUPS; sensor_group++) {
		if (sensor_data[(int) sensor_group] == 0) {
			continue;
		}

		char bit = 0;
		for (bit = 0; bit < SENSORS_PER_GROUP; bit++) {
			//sensor_data actually looks like 9,10,11,12,13,14,15,16,1,2,3,4,5,6,7,8
			if (!(sensor_data[(int) sensor_group] & (0x1 << bit))) {
				continue;
			}
            num_hit_sensor++;
			Sensor sensor;
			sensor.group = sensor_group;
			sensor.triggered_time = Time();
			if (bit + 1 <= 8) {
				sensor.id = 8 - bit;
			}
			else {
				sensor.id = 8 + 16 - bit;
			}
            sensor_output[num_hit_sensor] = sensor;
        }
    }
    return num_hit_sensor;
} 
	


