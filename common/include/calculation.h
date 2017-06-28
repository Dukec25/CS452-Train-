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

typedef struct sensor_distance {
    int sensor_id;
    int distance;
} sensor_distance;

#endif //__CALCULATION_H__
