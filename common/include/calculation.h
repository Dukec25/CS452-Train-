#ifndef __CALCULATION_H__
#define __CALCULATION_H__

#include <track_data.h>
#include <track_node.h>
#include <fifo.h>
#include <define.h>
#include <train.h>

int cal_distance(track_node *track, int src, int dest);
int choose_destination(track_node *track, int src, int dest, Train_server *train_server, Cli_request *update_request);
track_node* find_path(track_node *track, int src, int dest);
int switches_need_changes(int src, track_node *node, Train_server *train_server, Cli_request *update_request);

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
 *  Return the fifo which contains all the stops on the way of
 *  stop_distance + dest (excluding dest sensors and include the 
 *  sensor that is closest to the stop_location but not on the way)  
 *
 *  the first value in the fifo is the closest with the stop
 */
fifo_t find_stops_by_distance(track_node *track, int src, int dest, int stop_distance);

#endif //__CALCULATION_H__
