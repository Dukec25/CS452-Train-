#include <calculation.h>
#include <user_functions.h>
#include <cli.h>
#include <log.h>
#include <debug.h>

int choose_destination(track_node *track, int src, int dest, Train_server *train_server, Cli_request *update_request){
    dump(SUBMISSION, "src = %d, dest=%d \r\n", src, dest);
    track_node *temp;
    temp = find_path(track, src, dest);
    return switches_need_changes(src, temp, train_server, update_request);
}

int cal_distance(track_node *track, int src, int dest)
{
    track_node *temp;
    temp = find_path(track, src, dest);
    if (temp) {
        return temp->buf;
    }
    else{
        return 0;
    }
}

// TODO: deal with the case the input value is invalid 
track_node* find_path(track_node *track, int src, int dest)
{
    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX) {
        return NULL;
    }

    fifo_t queue; 
    fifo_init(&queue);

    track[src].buf = 0; // initialize the distance 
    fifo_put(&queue, &(track[src]));

    while (!is_fifo_empty(&queue)) {
        track_node *temp;
        fifo_get(&queue, &temp);

        if (temp->num == track[dest].num) {
            return temp;
        }

        if (temp->type == NODE_EXIT) {
            continue; 
        }
        else if (temp->type == NODE_BRANCH) {
            temp->edge[DIR_STRAIGHT].dest->buf = temp->buf + temp->edge[DIR_STRAIGHT].dist;
            temp->edge[DIR_STRAIGHT].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_STRAIGHT].dest);

            temp->edge[DIR_CURVED].dest->buf = temp->buf + temp->edge[DIR_CURVED].dist;
            temp->edge[DIR_CURVED].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_CURVED].dest);
        }
        else{
            temp->edge[DIR_AHEAD].dest->buf = temp->buf + temp->edge[DIR_AHEAD].dist;
            temp->edge[DIR_AHEAD].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_AHEAD].dest);
        }
    }

    return NULL;
}

int switches_need_changes(int src, track_node *node, Train_server *train_server, Cli_request *update_request){
    dump(SUBMISSION, "%s", "get into switches need change");
    int idx = 0; // br_update size is 10
    while(node->num != src){
        if(node->previous->type != NODE_BRANCH){
            node = node->previous;
            continue;
        } else {
            int node_id = node->previous->num;
            if(node->previous->edge[DIR_STRAIGHT].dest == node){
                if(train_server->switches_status[node_id-1] != STRAIGHT){
                    // update switches UI
                    update_request->br_update[idx].id = node_id;
                    update_request->br_update[idx++].state = 's';
                }
            } else{
                if(train_server->switches_status[node_id-1] != CURVE){
                    // update switches UI
                    update_request->br_update[idx].id = node_id;
                    update_request->br_update[idx++].state = 'c';
                }
            }
            node = node->previous;
        }
    }
    return idx;
}

int predict_next(track_node *track, int src, Train_server *train_server){
    track_node *temp = track[src].edge[DIR_AHEAD].dest;
    fifo_t queue; 
    fifo_init(&queue);

    fifo_put(&queue, temp);

    while(1){
        fifo_get(&queue, &temp);
        if(temp->type == NODE_SENSOR){
            return temp->num;
        } else if(temp->type == NODE_BRANCH){
            int cur_dir = train_server->switches_status[temp->num-1];
            if(cur_dir == STRAIGHT){
                fifo_put(&queue, temp->edge[DIR_STRAIGHT].dest);
            } else{
                fifo_put(&queue, temp->edge[DIR_CURVED].dest);
            }
        } else if(temp->type == NODE_MERGE){
           fifo_put(&queue, temp->edge[DIR_AHEAD].dest); 
        } else if(temp->type == NODE_EXIT){
            return -1;
        }

    }
    return -1;
}

int find_stops_by_distance(track_node *track, int src, int dest, int stop_distance, Sensor_dist* ans){
    track_node *node;
    node = find_path(track, src, dest);

    fifo_t queue; 
    fifo_init(&queue);
    fifo_put(&queue, node);

    int arr_len = 0;

    while(1){
        fifo_get(&queue, &node);
        node = node->previous;
        if(node->type == NODE_BRANCH){
            if(node->edge[DIR_STRAIGHT].dest == node){
                stop_distance -= node->edge[DIR_STRAIGHT].dist; 
            } else{
                stop_distance -= node->edge[DIR_CURVED].dist;
            }
        } else{
            stop_distance -= node->edge[DIR_AHEAD].dist;
        }

        fifo_put(&queue, node);

        if(node->type == NODE_SENSOR){
            Sensor_dist sensor_dist;
            sensor_dist.sensor_id = node->num;
            sensor_dist.distance = node->edge[DIR_AHEAD].dist;
            ans[arr_len++]  = sensor_dist;

            if(stop_distance <=0){
                return arr_len;
            }
        }
    }
}

