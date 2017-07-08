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

    /*cli_draw_trackA();*/
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

/*void cli_update_sensor(Sensor sensor, int last_sensor_update, int next_sensor_update)*/
/*{*/
	/*irq_save();*/

	/*Sensor last_sensor = num_to_sensor(last_sensor_update);*/
	/*int last_row = SENSOR_ROW + last_sensor.id * SENSOR_INDENT_HEIGHT;*/
	/*int last_col = SENSOR_COL + last_sensor.group * SENSOR_INDENT_WIDTH + SENSOR_LABEL_WIDTH;*/
	/*irq_pos(last_row, last_col);*/
	/*irq_printf(COM2, "%s", "  ");*/

	/*int row = SENSOR_ROW + sensor.id * SENSOR_INDENT_HEIGHT;*/
	/*int col = SENSOR_COL + sensor.group * SENSOR_INDENT_WIDTH + SENSOR_LABEL_WIDTH;*/
	/*irq_pos(row, col);*/
	/*irq_printf(COM2, "%s", "<-");*/

	/*Sensor next_sensor = num_to_sensor(next_sensor_update);*/
	/*irq_pos(SENSOR_PREDICTION_ROW, SENSOR_PREDICTION_COL);*/
	/*irq_printf(COM2, "Next Sensor: %c%s%d", SENSOR_LABEL_BASE + next_sensor.group, next_sensor.id < 10 ? "0" : "", next_sensor.id);*/

	/*irq_restore();*/
/*}*/

void cli_update_sensor(Sensor sensor, int last_sensor_update, int next_sensor_update, Map *map)
{
	irq_save();

    irq_pos(map->sensors[last_sensor_update].row, map->sensors[last_sensor_update].col);
    Putc(COM2, 'X');

    int cur_sensor = sensor_to_num(sensor);
    irq_pos(map->sensors[cur_sensor].row, map->sensors[cur_sensor].col);
    irq_printf(COM2, "\033[31m"); // make sensor display red
    Putc(COM2, 'X');
    irq_printf(COM2, "\033[0m"); // reset special format

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

void cli_draw_trackA(Map *map_a){
    map_a->ascii =  ""
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
        "-X------X---O         --X---O-----X---X-----O--X--X\n"
        "             \\               \\             /\n"
        "-X--------X---O---------X-----O-----------O----X--------\n"
        ;
    int col_idx=0;
    int map_first_row = MAP_FIRST_ROW;
    // draw the track 
    irq_pos(SENSOR_ROW, SENSOR_COL);
    int idx = 0;
    while(1){
        if(map_a->ascii[idx] == '\n'){
            cli_next_line();
        } else if (map_a->ascii[idx] == 0){
            break;
        } else{
            Putc(COM2, map_a->ascii[idx]);
        }
        idx++;
    }
    // row 0
    col_idx = 8; // A1,2
    map_a->sensors[0].row = map_first_row;
    map_a->sensors[0].col = col_idx;
    map_a->sensors[1].row = map_first_row;
    map_a->sensors[1].col = col_idx;
    col_idx+=5;  // 12
    map_a->switches[12].row = map_first_row;
    map_a->switches[12].col = col_idx;
    col_idx+=10; // 11
    map_a->switches[11].row = map_first_row;
    map_a->switches[11].col = col_idx;
    col_idx += 8; // C13,14
    map_a->sensors[44].row = map_first_row;
    map_a->sensors[44].col = col_idx;
    map_a->sensors[45].row = map_first_row;
    map_a->sensors[45].col = col_idx;
    col_idx += 12; // E7,8
    map_a->sensors[70].row = map_first_row;
    map_a->sensors[70].col = col_idx;
    map_a->sensors[71].row = map_first_row;
    map_a->sensors[71].col = col_idx;
    // row 1 
    col_idx = 52; // D7,8
    map_a->sensors[54].row = map_first_row+1;
    map_a->sensors[54].col = col_idx;
    map_a->sensors[55].row = map_first_row+1;
    map_a->sensors[55].col = col_idx;
    
    // row 2
    col_idx = 6; // A13, 14
    map_a->sensors[12].row = map_first_row+2;
    map_a->sensors[12].col = col_idx;
    map_a->sensors[13].row = map_first_row+2;
    map_a->sensors[13].col = col_idx;
    col_idx+=5; // 4
    map_a->switches[4].row = map_first_row+2;
    map_a->switches[4].col = col_idx;
    col_idx+=10; // 14 
    map_a->switches[14].row = map_first_row+2;
    map_a->switches[14].col = col_idx;
    col_idx += 5; // C11, 12
    map_a->sensors[42].row = map_first_row+2;
    map_a->sensors[42].col = col_idx;
    map_a->sensors[43].row = map_first_row+2;
    map_a->sensors[43].col = col_idx;
    col_idx+=7; // 13 
    map_a->switches[13].row = map_first_row+2;
    map_a->switches[13].col = col_idx;
    col_idx += 3; // B5, 6
    map_a->sensors[20].row = map_first_row+2;
    map_a->sensors[20].col = col_idx;
    map_a->sensors[21].row = map_first_row+2;
    map_a->sensors[21].col = col_idx;
    col_idx += 2; // D3, 4
    map_a->sensors[50].row = map_first_row+2;
    map_a->sensors[50].col = col_idx;
    map_a->sensors[51].row = map_first_row+2;
    map_a->sensors[51].col = col_idx;
    col_idx+=3; // 10
    map_a->switches[10].row = map_first_row+2;
    map_a->switches[10].col = col_idx;
    col_idx += 4; // E5, 6
    map_a->sensors[68].row = map_first_row+2;
    map_a->sensors[68].col = col_idx;
    map_a->sensors[69].row = map_first_row+2;
    map_a->sensors[69].col = col_idx;
    col_idx += 4; // D5, 6
    map_a->sensors[52].row = map_first_row+2;
    map_a->sensors[52].col = col_idx;
    map_a->sensors[53].row = map_first_row+2;
    map_a->sensors[53].col = col_idx;
    col_idx+=4; // 9
    map_a->switches[9].row = map_first_row+2;
    map_a->switches[9].col = col_idx;
    
    // row 3
    col_idx = 34; // E15, 16
    map_a->sensors[78].row = map_first_row+3;
    map_a->sensors[78].col = col_idx;
    map_a->sensors[79].row = map_first_row+3;
    map_a->sensors[79].col = col_idx;
    col_idx += 6; // E3, 4 
    map_a->sensors[66].row = map_first_row+3;
    map_a->sensors[66].col = col_idx;
    map_a->sensors[67].row = map_first_row+3;
    map_a->sensors[67].col = col_idx;
    // row 4 
    col_idx = 4; // A15, 16
    map_a->sensors[14].row = map_first_row+4;
    map_a->sensors[14].col = col_idx;
    map_a->sensors[15].row = map_first_row+4;
    map_a->sensors[15].col = col_idx;
    col_idx += 15; // A3, 4
    map_a->sensors[2].row = map_first_row+4;
    map_a->sensors[2].col = col_idx;
    map_a->sensors[3].row = map_first_row+4;
    map_a->sensors[3].col = col_idx;
    col_idx += 16; // E1, 2
    map_a->sensors[64].row = map_first_row+4;
    map_a->sensors[64].col = col_idx;
    map_a->sensors[65].row = map_first_row+4;
    map_a->sensors[65].col = col_idx;
    col_idx += 4; // D1, 2
    map_a->sensors[48].row = map_first_row+4;
    map_a->sensors[48].col = col_idx;
    map_a->sensors[49].row = map_first_row+4;
    map_a->sensors[49].col = col_idx;
    // row 5
    col_idx = 36; // 22
    map_a->switches[22].row = map_first_row+5;
    map_a->switches[22].col = col_idx;
    col_idx += 2; // 21
    map_a->switches[21].row = map_first_row+5;
    map_a->switches[21].col = col_idx;
    // row 7
    col_idx = 36; // 19
    map_a->switches[19].row = map_first_row+7;
    map_a->switches[19].col = col_idx;
    col_idx += 2; // 20
    map_a->switches[20].row = map_first_row+7;
    map_a->switches[20].col = col_idx;
    // row 8
    col_idx = 2; // A11, 12
    map_a->sensors[10].row = map_first_row+8;
    map_a->sensors[10].col = col_idx;
    map_a->sensors[11].row = map_first_row+8;
    map_a->sensors[11].col = col_idx;
    col_idx += 17; // B15, 16
    map_a->sensors[30].row = map_first_row+8;
    map_a->sensors[30].col = col_idx;
    map_a->sensors[31].row = map_first_row+8;
    map_a->sensors[31].col = col_idx;
    col_idx += 16; // C1, 2
    map_a->sensors[32].row = map_first_row+8;
    map_a->sensors[32].col = col_idx;
    map_a->sensors[33].row = map_first_row+8;
    map_a->sensors[33].col = col_idx;
    col_idx += 4; // B13, 14
    map_a->sensors[28].row = map_first_row+8;
    map_a->sensors[28].col = col_idx;
    map_a->sensors[29].row = map_first_row+8;
    map_a->sensors[29].col = col_idx;
    // row 9
    // B3, B4
    col_idx = 34;
    map_a->sensors[18].row = map_first_row+9;
    map_a->sensors[18].col = col_idx;
    map_a->sensors[19].row = map_first_row+9;
    map_a->sensors[19].col = col_idx;
    // D15, D16
    col_idx += 8;
    map_a->sensors[62].row = map_first_row+9;
    map_a->sensors[62].col = col_idx;
    map_a->sensors[63].row = map_first_row+9;
    map_a->sensors[63].col = col_idx;
    // row 10 
    col_idx = 2;
    //B7, 8
    map_a->sensors[22].row = map_first_row+10;
    map_a->sensors[22].col = col_idx;
    map_a->sensors[23].row = map_first_row+10;
    map_a->sensors[23].col = col_idx;
    //A9, 10
    col_idx+=5;
    map_a->sensors[8].row = map_first_row+10;
    map_a->sensors[8].col = col_idx;
    map_a->sensors[9].row = map_first_row+10;
    map_a->sensors[9].col = col_idx;
    col_idx+=4;
    map_a->switches[1].row = map_first_row+10;
    map_a->switches[1].col = col_idx;
    col_idx+=10;
    map_a->switches[15].row = map_first_row+10;
    map_a->switches[15].col = col_idx;
    //C9,10
    col_idx+=4;
    map_a->sensors[40].row = map_first_row+10;
    map_a->sensors[40].col = col_idx;
    map_a->sensors[41].row = map_first_row+10;
    map_a->sensors[41].col = col_idx;
    col_idx+=6;
    map_a->switches[16].row = map_first_row+10;
    map_a->switches[16].col = col_idx;
    //B1, 2
    col_idx+=3;
    map_a->sensors[16].row = map_first_row+10;
    map_a->sensors[16].col = col_idx;
    map_a->sensors[17].row = map_first_row+10;
    map_a->sensors[17].col = col_idx;
    //D13, 14
    col_idx+=6;
    map_a->sensors[60].row = map_first_row+10;
    map_a->sensors[60].col = col_idx;
    map_a->sensors[61].row = map_first_row+10;
    map_a->sensors[61].col = col_idx;
    col_idx+=3;
    map_a->switches[17].row = map_first_row+10;
    map_a->switches[17].col = col_idx;
    //E13, 14
    col_idx+=2;
    map_a->sensors[76].row = map_first_row+10;
    map_a->sensors[76].col = col_idx;
    map_a->sensors[77].row = map_first_row+10;
    map_a->sensors[77].col = col_idx;
    //E9, 10
    col_idx+=4;
    map_a->sensors[72].row = map_first_row+10;
    map_a->sensors[72].col = col_idx;
    map_a->sensors[73].row = map_first_row+10;
    map_a->sensors[73].col = col_idx;
    col_idx+=4;
    map_a->switches[8].row = map_first_row+10;
    map_a->switches[8].col = col_idx;
    // row 12
    col_idx = 2; //B11, B12
    map_a->sensors[26].row = map_first_row+12;
    map_a->sensors[26].col = col_idx;
    map_a->sensors[27].row = map_first_row+12;
    map_a->sensors[27].col = col_idx;
    col_idx+=7; // A7, 8
    map_a->sensors[6].row = map_first_row+12;
    map_a->sensors[6].col = col_idx;
    map_a->sensors[7].row = map_first_row+12;
    map_a->sensors[7].col = col_idx;
    col_idx+=4; // 2
    map_a->switches[2].row = map_first_row+12;
    map_a->switches[2].col = col_idx;
    col_idx+=12; // C5, C6
    map_a->sensors[36].row = map_first_row+12;
    map_a->sensors[36].col = col_idx;
    map_a->sensors[37].row = map_first_row+12;
    map_a->sensors[37].col = col_idx;
    col_idx+=4; // 6
    map_a->switches[6].row = map_first_row+12;
    map_a->switches[6].col = col_idx;
    col_idx+=6; // C15, 16
    map_a->sensors[46].row = map_first_row+12;
    map_a->sensors[46].col = col_idx;
    map_a->sensors[47].row = map_first_row+12;
    map_a->sensors[47].col = col_idx;
    col_idx+=4; // D11, 12
    map_a->sensors[58].row = map_first_row+12;
    map_a->sensors[58].col = col_idx;
    map_a->sensors[59].row = map_first_row+12;
    map_a->sensors[59].col = col_idx;
    col_idx+=6; // 7
    map_a->switches[7].row = map_first_row+12;
    map_a->switches[7].col = col_idx;
    col_idx+=3; // E11, 12
    map_a->sensors[74].row = map_first_row+12;
    map_a->sensors[74].col = col_idx;
    map_a->sensors[75].row = map_first_row+12;
    map_a->sensors[75].col = col_idx;
    col_idx+=3; // D9, 10
    map_a->sensors[56].row = map_first_row+12;
    map_a->sensors[56].col = col_idx;
    map_a->sensors[57].row = map_first_row+12;
    map_a->sensors[57].col = col_idx;
    
    // row 14
    col_idx = 2;
    //B9, 10
    map_a->sensors[24].row = map_first_row+14;
    map_a->sensors[24].col = col_idx;
    map_a->sensors[25].row = map_first_row+14;
    map_a->sensors[25].col = col_idx;
    //A5, 6
    col_idx += 9;
    map_a->sensors[4].row = map_first_row+14;
    map_a->sensors[4].col = col_idx;
    map_a->sensors[5].row = map_first_row+14;
    map_a->sensors[5].col = col_idx;
    col_idx +=4; // 3
    map_a->switches[3].row = map_first_row+14;
    map_a->switches[3].col = col_idx;
    // C7, 8
    col_idx += 10;
    map_a->sensors[38].row = map_first_row+14;
    map_a->sensors[38].col = col_idx;
    map_a->sensors[39].row = map_first_row+14;
    map_a->sensors[39].col = col_idx;
    col_idx += 6; // 6
    map_a->switches[18].row = map_first_row+14;
    map_a->switches[18].col = col_idx;
    col_idx +=12; // 12
    map_a->switches[5].row = map_first_row+14;
    map_a->switches[5].col = col_idx;
    // C3, 4 
    col_idx += 5;
    map_a->sensors[34].row = map_first_row+14;
    map_a->sensors[34].col = col_idx;
    map_a->sensors[35].row = map_first_row+14;
    map_a->sensors[35].col = col_idx;
}

void cli_draw_trackB(Map *map_b){
    map_b->ascii = ""
        "----------X----O---------O----X-------O----X---------X--\n"
        "              /           \\            \\\n"
        "      ---X---O---X-----X---O---X---     O----X-------X--\n"
        "     X                             \\     \\\n"
        "    O---X---X---O-X---X-O-----X-----O     O----X-----X--\n"
        "   /             X     X             \\     \\\n"
        "  /               X | X               X     X\n"
        " |                 O|O                 |     |\n"
        " |                  |                  |     |\n"
        "                   O|O                 |     |\n"
        " |\\               X | X               X     X\n"
        " |               X     X             /     /\n"
        " O-------X--X---O-X---X-O-----X-----O     O----X--------\n"
        "  X                                /     /\n"
        "   -------X----------------X------O-----O----X----------\n"
        ;

    int col_idx=0;
    int map_first_row = MAP_FIRST_ROW;
    // draw the track 
    irq_pos(SENSOR_ROW, SENSOR_COL);
    int idx = 0;
    while(1){
        if(map_b->ascii[idx] == '\n'){
            cli_next_line();
        } else if (map_b->ascii[idx] == 0){
            break;
        } else{
            Putc(COM2, map_b->ascii[idx]);
        }
        idx++;
    }

    // Row 12
    col_idx = 2; // 9
    map_b->switches[9].row = map_first_row+12;
    map_b->switches[9].col = col_idx;
    col_idx += 8; // D5, 6
    map_b->sensors[52].row = map_first_row+12;
    map_b->sensors[52].col = col_idx;
    map_b->sensors[53].row = map_first_row+12;
    map_b->sensors[53].col = col_idx;
    col_idx += 3; // E5, 6
    map_b->sensors[68].row = map_first_row+12;
    map_b->sensors[68].col = col_idx;
    map_b->sensors[69].row = map_first_row+12;
    map_b->sensors[69].col = col_idx;
    col_idx += 4; // 10
    map_b->switches[10].row = map_first_row+12;
    map_b->switches[10].col = col_idx;
    col_idx += 2; // D3, 4
    map_b->sensors[50].row = map_first_row+12;
    map_b->sensors[50].col = col_idx;
    map_b->sensors[51].row = map_first_row+12;
    map_b->sensors[51].col = col_idx;
    col_idx += 4; // B5, 6
    map_b->sensors[20].row = map_first_row+12;
    map_b->sensors[20].col = col_idx;
    map_b->sensors[21].row = map_first_row+12;
    map_b->sensors[21].col = col_idx;
    col_idx += 2; // 13
    map_b->switches[13].row = map_first_row+12;
    map_b->switches[13].col = col_idx;
    col_idx += 6; // C11, 12
    map_b->sensors[42].row = map_first_row+12;
    map_b->sensors[42].col = col_idx;
    map_b->sensors[43].row = map_first_row+12;
    map_b->sensors[43].col = col_idx;
    col_idx += 6; // 14
    map_b->switches[14].row = map_first_row+12;
    map_b->switches[14].col = col_idx;
    col_idx += 6; // 4
    map_b->switches[4].row = map_first_row+12;
    map_b->switches[4].col = col_idx;
    col_idx += 5; // A13, 14
    map_b->sensors[12].row = map_first_row+12;
    map_b->sensors[12].col = col_idx;
    map_b->sensors[13].row = map_first_row+12;
    map_b->sensors[13].col = col_idx;

    // Row 13
    col_idx = 3; // D7, 8
    map_b->sensors[54].row = map_first_row+13;
    map_b->sensors[54].col = col_idx;
    map_b->sensors[55].row = map_first_row+13;
    map_b->sensors[55].col = col_idx;

    // Row 14
    col_idx = 10; // E7, 8
    map_b->sensors[70].row = map_first_row+14;
    map_b->sensors[70].col = col_idx;
    map_b->sensors[71].row = map_first_row+14;
    map_b->sensors[71].col = col_idx;
    col_idx += 17; // C13, 14
    map_b->sensors[44].row = map_first_row+14;
    map_b->sensors[44].col = col_idx;
    map_b->sensors[45].row = map_first_row+14;
    map_b->sensors[45].col = col_idx;
    col_idx += 7; // 11
    map_b->switches[11].row = map_first_row+14;
    map_b->switches[11].col = col_idx;
    col_idx += 6; // 12
    map_b->switches[12].row = map_first_row+14;
    map_b->switches[12].col = col_idx;
    col_idx += 5; // A1, 2
    map_b->sensors[0].row = map_first_row+14;
    map_b->sensors[0].col = col_idx;
    map_b->sensors[1].row = map_first_row+14;
    map_b->sensors[1].col = col_idx;
}

// use busy wait 
cli_next_line(){
    irq_printf(COM2, "\033E"); 
}
