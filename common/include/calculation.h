#ifndef __CALCULATION_H__
#define __CALCULATION_H__
#include <track_node.h>
#include <train_task.h>
#include <string.h>
#include <lifo.h>

/*
 * the base function of most the functions listed below, used to return a
 * path from src and dest  
 */
track_node* find_path_with_blocks(track_node *track, int src, int dest, int *resource);

/*
 * calculate the physical distance between src and dest in um
 */
int cal_distance(track_node *track, int src, int dest);

/*
 * helper method of choose_destination
 */
int switches_need_changes(int src, track_node *node, Train_server *train_server, Br_lifo *br_lifo_struct);

/*
 * Used to predict next sensor stop based on src
 * Returns -1 if the road ends 
 * Returns other value in normal case.
 */
int predict_next(track_node *track, int src, Train_server *train_server);

void put_cmd_fifo(track_node *track, int dest, int *resource, track_node *node, Train *train, 
        TS_request *ts_request);

track_node* find_path(track_node *track, int src, int dest);
typedef struct Sensor_dist {
    int sensor_id;
    int distance; // in um
} Sensor_dist;

/*
 *  Return the length of the Sensor_dist array, the first value in the 
 *  array is the sensor that is the closest with stop 
 *  
 *  including the sensors on the way of stop_distance + dest (excluding dest sensors and include the 
 *  sensor that is closest to the stop_location but not on the way)  
 */
int find_stops_by_distance(track_node *track, int src, int dest, int stop_distance, Sensor_dist* ans, int *resource);

// those are here right now just for testing purpose, can be deleted later 
void generate_cmds_table(track_node *track, Lifo_t *parsing_table, int reverse, Train *train, 
        TS_request *ts_request);
void calculate_park(track_node *node, Train *train, Park_info *park_info);
#endif //__CALCULATION_H__
