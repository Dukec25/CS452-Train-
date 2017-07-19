#ifndef __CALCULATION_H__
#define __CALCULATION_H__
#include <track_node.h>
#include <train_task.h>
#include <string.h>

/*
 * the base function of most the functions listed below, used to return a
 * path from src and dest  
 */
track_node* find_path(track_node *track, int src, int dest);
track_node* find_path_with_blocks(track_node *track, int src, int dest, int *resource);

/*
 * calculate the physical distance between src and dest in um
 */
int cal_distance(track_node *track, int src, int dest);

/*
 * function used to flip switches in order to get the train to dest
 */
int choose_destination(int src, int dest, Train_server *train_server, Br_lifo *br_lifo_struct);

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
int find_stops_by_distance(track_node *track, int src, int dest, int stop_distance, Sensor_dist* ans, int *resource,
        int *reverse);

#endif //__CALCULATION_H__
