#include <cursor.h>
#include <cli.h>
#include <train.h>
#include <clock.h>
#include <string.h>
#include <bwio.h>
#include <irq_io.h>

void cli_startup()
{
	int row, col;

	// clear screen
	bw_cls();


	// draw borders
	for (col = LEFT_BORDER + 1; col <= RIGHT_BORDER - 1; col++) {
		// upper border
		bw_pos(UPPER_BORDER, col);
		/*bwputc(COM2, '-');*/
		// status border
		bw_pos(STATUS_BORDER, col);
		/*bwputc(COM2, '-');*/
		// label border
		bw_pos(LABEL_BORDER, col);
		/*bwputc(COM2, '-');*/
		// bottom borders
		bw_pos(BOTTOM_BORDER, col);
        /*bwputc(COM2, '-');*/
	}
	// left, middle, and right borders
	/*for (row = UPPER_BORDER; row <= BOTTOM_BORDER; row++) {*/
		/*bw_pos(row, LEFT_BORDER);*/
		/*bwputc(COM2, '|');*/
		/*bw_pos(row, MIDDLE_BORDER);*/
		/*bwputc(COM2, '|');*/
		/*bw_pos(row, RIGHT_BORDER);*/
		/*bwputc(COM2, '|');*/
	/*}*/

	// Place clock
	bw_pos(CLOCK_ROW, CLOCK_COL);
	bwputstr(COM2, "000:00:0");

	// Place train
	bw_pos(TRAIN_ROW, TRAIN_COL);
	bwputstr(COM2, "tr 00 00");

	// Place sensors
	bw_pos(SENSOR_LABEL_ROW, SENSOR_COL);
	bwputstr(COM2, "Track Map");
	int sensor_num;
    bwprintf(COM2, "\033[32m"); // make sensor display green
	/*for (sensor_num = 0; sensor_num < SENSOR_GROUPS * SENSORS_PER_GROUP; sensor_num++) {*/
		/*Sensor sensor = num_to_sensor(sensor_num);*/
		/*int row = SENSOR_ROW + sensor.id * SENSOR_INDENT_HEIGHT;*/
		/*int col = SENSOR_COL + sensor.group * SENSOR_INDENT_WIDTH;*/
		/*bw_pos(row, col);*/
		/*bwprintf(COM2, "%c%s%d", SENSOR_LABEL_BASE + sensor.group, sensor.id < 10 ? "0" : "", sensor.id);*/
	/*}*/
    bwprintf(COM2, "\033[0m"); // reset special format

	// Place switches
	bw_pos(SWITCH_LABEL_ROW, SWITCH_COL);
	bwputstr(COM2, "Switches");
	int sw;
    bwprintf(COM2, "\033[36m"); // make switches display cyan
	/*for (sw = 0; sw < NUM_SWITCHES; sw++) {*/
		/*int sw_address = switch_id_to_byte(sw + 1);*/
		/*// Place sw id*/
		/*bw_pos(SWITCH_ROW + sw, SWITCH_COL);*/
		/*bwputx(COM2, sw_address);*/
	/*}*/
    bwprintf(COM2, "\033[0m"); // reset special format

    cli_draw_track();
	// Place input cursor at the end
	bw_pos(HEIGHT, 0);
	bwputstr(COM2, "> ");

	// Save screen setup
	bw_save();
}

void cli_track_startup()
{
	bw_save();

	int row, col;

	// draw borders
	for (col = TRACK_DATA_LEFT_BORDER + 1; col <= TRACK_DATA_RIGHT_BORDER - 1; col++) {
		// upper border
		bw_pos(TRACK_DATA_UPPER_BORDER, col);
		bwputc(COM2, '-');
		// status border
		bw_pos(TRACK_DATA_STATUS_BORDER, col);
		bwputc(COM2, '-');
		// bottom borders
		bw_pos(TRACK_DATA_BOTTOM_BORDER, col);
		bwputc(COM2, '-');
	}
	// left, middle, and right borders
	for (row = TRACK_DATA_UPPER_BORDER; row <= TRACK_DATA_BOTTOM_BORDER; row++) {
		bw_pos(row, TRACK_DATA_LEFT_BORDER);
		bwputc(COM2, '|');
		bw_pos(row, TRACK_DATA_RIGHT_BORDER);
		bwputc(COM2, '|');
	}

	// Place trackA
	bw_pos(TRACK_DATA_STATUS_BORDER - 1, TRACK_DATA_LABEL_COL);
	bwputstr(COM2, "Track A (mm, cm/sec)");
	bw_restore();
}

void cli_user_input(Command_buffer *command_buffer)
{
    /*irq_pos(HEIGHT, 0);*/
	command_buffer->data[command_buffer->pos] = '\0';
    /*irq_printf(COM2, "> %s", command_buffer->data);*/
    bwprintf(COM2, "\r\n");
}

void cli_update_clock(Clock clock)
{
	irq_save();
	// Place clock
	irq_pos(CLOCK_ROW, CLOCK_COL);
	irq_printf(COM2, "%s%d:%s%d:%d",
					 clock.min < 100 ? (clock.min < 10 ? "00" : "0") : "",
					 clock.min,
					 clock.sec < 10 ? "0" : "",
					 clock.sec,
					 clock.tenth_sec);
	irq_restore();
}

void cli_update_train(Train train)
{
	irq_save();
	irq_pos(TRAIN_ROW, TRAIN_COL);
	irq_printf(COM2, "tr %d %s%d", train.id, train.speed < 10 ? "0" : "", train.speed);
	irq_restore();
}

void cli_update_switch(Switch sw)
{
	irq_save();
	irq_pos(SWITCH_ROW + sw.id - 1, RIGHT_BORDER - 1);
	Putc(COM2, toupper(sw.state));
	irq_restore();
}

void cli_update_sensor(Sensor sensor, int last_sensor_update, int next_sensor_update)
{
	irq_save();

	Sensor last_sensor = num_to_sensor(last_sensor_update);
	int last_row = SENSOR_ROW + last_sensor.id * SENSOR_INDENT_HEIGHT;
	int last_col = SENSOR_COL + last_sensor.group * SENSOR_INDENT_WIDTH + SENSOR_LABEL_WIDTH;
	irq_pos(last_row, last_col);
	irq_printf(COM2, "%s", "  ");

	int row = SENSOR_ROW + sensor.id * SENSOR_INDENT_HEIGHT;
	int col = SENSOR_COL + sensor.group * SENSOR_INDENT_WIDTH + SENSOR_LABEL_WIDTH;
	irq_pos(row, col);
	irq_printf(COM2, "%s", "<-");

	Sensor next_sensor = num_to_sensor(next_sensor_update);
	irq_pos(SENSOR_PREDICTION_ROW, SENSOR_PREDICTION_COL);
	irq_printf(COM2, "Next Sensor: %c%s%d", SENSOR_LABEL_BASE + next_sensor.group, next_sensor.id < 10 ? "0" : "", next_sensor.id);

	irq_restore();
}

void cli_update_track(Calibration_package calibration_pkg, int updates)
{
	if (calibration_pkg.src == -1) {
		return;
	}
	irq_save();
	irq_printf(COM2, "\033[33m");
	Sensor src = num_to_sensor(calibration_pkg.src);
	Sensor dest = num_to_sensor(calibration_pkg.dest);
    irq_pos(updates % 22 + 1, TRACK_DATA_COL + updates / 22 % 6 * TRACK_DATA_LENGTH);
	/*irq_pos(updates % HEIGHT + 1, TRACK_DATA_COL);	*/
	irq_printf(COM2, "%c%d->%c%d,%d,%d[10ms],%d[10ms]",
				src.group + SENSOR_LABEL_BASE, src.id, dest.group + SENSOR_LABEL_BASE, dest.id,
				calibration_pkg.distance, calibration_pkg.time, calibration_pkg.velocity);
    irq_printf(COM2, "\033[0m"); // reset special format
	irq_restore();
}

void cli_draw_track(){
    Map map_a;
    map_a.ascii =  ""
        "-------X----O---------O-------X-----------X--------\n"
        "           /         /                             X\n"
        "-----X----O         O----X------O--X-X--O---X---X---O\n"
        "         /         /             X     X             \\\n"
        "---X-----         X               X | X               \\\n"
        "                 |                 O|O                 |\n"
        "                 |                  |                  |\n"
        "                 |                 O|O                 |\n"
        "-X-------         X               X | X               /\n"
        "         \\         \\          X       X              /\n"
        "-X----X---O         O---X-----O--X-----X--O-X---X---O\n"
        "           \\         \\                             /\n"
        "-X------X---O         --X---O-----X---X-----O--X---\n"
        "             \\               \\             /\n"
        "-X--------X---O---------X-----O-----------O----X--------\n"
        ;
    int col_idx=0;

    // draw the track 
	bw_pos(SENSOR_ROW, SENSOR_COL);
    int idx = 0;

    while(1){
        if(map_a.ascii[idx] == '\n'){
            cli_next_line();
        } else if (map_a[idx] == 0){
            break;
        } else{
            bwputc(COM2, map_a[idx]);
        }
        idx++;
    }
    // row 9
    // B3, B4
    col_idx = 33;
    map_a.sensors[18].row = 9;
    map_a.sensors[18].col = col_idx;
    map_a.sensors[19].row = 9;
    map_a.sensors[19].col = col_idx;
    // D15, D16
    col_idx += 8;
    map_a.sensors[62].row = 9;
    map_a.sensors[62].col = col_idx;
    map_a.sensors[63].row = 9;
    map_a.sensors[63].col = col_idx;

    // row 10 
    col_idx = 1;
    //B7, 8
    map_a.sensors[22].row = 10;
    map_a.sensors[22].col = col_idx;
    map_a.sensors[23].row = 10;
    map_a.sensors[23].col = col_idx;
    //A9, 10
    col_idx+=5;
    map_a.sensors[8].row = 10;
    map_a.sensors[8].col = col_idx;
    map_a.sensors[9].row = 10;
    map_a.sensors[9].col = col_idx;
    col_idx+=4;
    map_a.switches[1].row = 10;
    map_a.switches[1].col = col_idx;
    col_idx+=10;
    map_a.switches[15].row = 10;
    map_a.switches[15].col = col_idx;
    //C9, 10
    col_idx+=4;
    map_a.sensors[40].row = 10;
    map_a.sensors[40].col = col_idx;
    map_a.sensors[41].row = 10;
    map_a.sensors[41].col = col_idx;
    col_idx+=6;
    map_a.switches[16].row = 10;
    map_a.switches[16].col = col_idx;
    //B1, 2
    col_idx+=3;
    map_a.sensors[16].row = 10;
    map_a.sensors[16].col = col_idx;
    map_a.sensors[17].row = 10;
    map_a.sensors[17].col = col_idx;
    //D13, 14
    col_idx+=6;
    map_a.sensors[60].row = 10;
    map_a.sensors[60].col = col_idx;
    map_a.sensors[61].row = 10;
    map_a.sensors[61].col = col_idx;
    col_idx+=3;
    map_a.switches[17].row = 10;
    map_a.switches[17].col = col_idx;
    //E13, 14
    col_idx+=2;
    map_a.sensors[76].row = 10;
    map_a.sensors[76].col = col_idx;
    map_a.sensors[77].row = 10;
    map_a.sensors[77].col = col_idx;
    //E9, 10
    col_idx+=4;
    map_a.sensors[72].row = 10;
    map_a.sensors[72].col = col_idx;
    map_a.sensors[73].row = 10;
    map_a.sensors[73].col = col_idx;
    col_idx+=4;
    map_a.switches[8].row = 10;
    map_a.switches[8].col = col_idx;


    // row 12
    col_idx = 1;
    // B11, B12
    map_a.sensors[26].row = 12;
    map_a.sensors[26].col = col_idx;
    map_a.sensors[27].row = 12;
    map_a.sensors[27].col = col_idx;
    // A7, A8
    map_a.sensors[6].row = 12;
    map_a.sensors[6].col = col_idx+7;
    map_a.sensors[7].row = 12;
    map_a.sensors[7].col = col_idx+7;
    col_idx+=7;
    map_a.switches[2].row = 12;
    map_a.switches[2].col = col_idx+4;
    col_idx+=4;
    // C5, C6
    map_a.sensors[36].row = 12;
    map_a.sensors[36].col = col_idx+12;
    map_a.sensors[37].row = 12;
    map_a.sensors[37].col = col_idx+12;
    col_idx+=12;
    map_a.switches[6].row = 12;
    map_a.switches[6].col = col_idx+4;
    col_idx+=4;
    // C15, C16
    map_a.sensors[46].row = 12;
    map_a.sensors[46].col = col_idx+6;
    map_a.sensors[47].row = 12;
    map_a.sensors[47].col = col_idx+6;
    col_idx+=6;
    // D11, D12
    map_a.sensors[58].row = 12;
    map_a.sensors[58].col = col_idx+4;
    map_a.sensors[59].row = 12;
    map_a.sensors[59].col = col_idx+4;
    col_idx+=4;
    map_a.switches[7].row = 12;
    map_a.switches[7].col = col_idx+6;
    col_idx+=6;
    // E11, E12
    map_a.sensors[74].row = 12;
    map_a.sensors[74].col = col_idx+3;
    map_a.sensors[75].row = 12;
    map_a.sensors[75].col = col_idx+3;
    
    // row 13 
    // row 14
    col_idx = 1;
    //B9, 10
    map_a.sensors[24].row = 14;
    map_a.sensors[24].col = 1;
    map_a.sensors[25].row = 14;
    map_a.sensors[25].col = 1;
    //A5, 6
    map_a.sensors[4].row = 14;
    map_a.sensors[4].col = col_idx+9;
    map_a.sensors[5].row = 14;
    map_a.sensors[5].col = col_idx+9;
    col_idx += 9;
    map_a.switches[3].row = 14;
    map_a.switches[3].col = col_idx + 4;
    col_idx +=4;
    // C7, 8
    map_a.sensors[38].row = 14;
    map_a.sensors[38].col = col_idx+10;
    map_a.sensors[39].row = 14;
    map_a.sensors[39].col = col_idx+10;
    col_idx += 10;
    map_a.switches[18].row = 14;
    map_a.switches[18].col = col_idx + 6;
    col_idx += 6;
    map_a.switches[5].row = 14;
    map_a.switches[5].col = col_idx + 12;
    col_idx + =12;
    // C3, 4 
    map_a.switches[34].row = 14;
    map_a.switches[34].col = col_idx + 5;
    map_a.switches[35].row = 14;
    map_a.switches[35].col = col_idx + 5;

    


    char* map_b;
    map_b = ""
        "----------X----O---------O----X-------O----X---------X--\n"
        "              /           \\            \\\n"
        "      ---X---O---X-----X---O---X---     O----X-------X--\n"
        "     X                             \\     \\\n"
        "    O---X---X---O-X---X-O-----X-----O     O----X-----X--\n"
        "   /             X     X             \\     \\\n"
        "  /               X | X               X     X\n"
        " |                 O|O                 |     |\n"
        " |                  |                  |     |\n"
        " O                 O|O                 |     |\n"
        " |\\               X | X               X     X\n"
        " | X             X     X             /     /\n"
        " |  ------X-----O-X---X-O-----X-----O     O----X--------\n"
        "  X                                /     /\n"
        "   -------X----------------X------O-----O----X----------\n"
        ;
}

// use busy wait 
cli_next_line(){
    bwprintf(COM2, "\033E"); 
}
