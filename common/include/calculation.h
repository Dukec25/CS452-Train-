#ifndef __CALCULATION_H__
#define __CALCULATION_H__

#include <track_data.h>
#include <track_node.h>
#include <fifo.h>
#include <define.h>
#include <train.h>

int cal_distance(track_node *track, int src, int dest);
void choose_destination(track_node *track, int src, int dest, Train_server *train_server);
track_node* find_path(track_node *track, int src, int dest);
void switches_need_changes(int src, track_node *node, Train_server *train_server);

#endif //__CALCULATION_H__
