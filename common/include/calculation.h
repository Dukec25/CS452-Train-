#ifndef __CALCULATION_H__
#define __CALCULATION_H__

#include <track_data.h>
#include <track_node.h>
#include <fifo.h>
#include <define.h>
#include <train.h>

int cal_distance(track_node *track, int src, int dest);
int choose_destination(track_node *track, int src, int dest, Train_server *train_server);
track_node* find_path(track_node *track, int src, int dest);
int switches_need_changes(int src, track_node *node, Train_server *train_server);

/*
 * Returns -1 if the road ends 
 * Returns other value in normal case.
 */
int predict_next(track_node *track, int src, Train_server *train_server);

typedef struct Sensor_dist {
    int sensor_id;
    int distance;
} Sensor_dist;


/*
 *  Return the length of the Sensor_dist array, the first value in the 
 *  array is the sensor that is the closest with stop 
 *  
 *  including the sensors on the way of stop_distance + dest (excluding dest sensors and include the 
 *  sensor that is closest to the stop_location but not on the way)  
 */
int find_stops_by_distance(track_node *track, int src, int dest, int stop_distance, Sensor_dist* ans);

#endif //__CALCULATION_H__
