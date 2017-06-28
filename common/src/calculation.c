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

int switches_need_changes(int src, track_node *node, Train_server *train_server, Cli_request *update_request, ){
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
                    // flip the switches 
                    irq_printf(COM1, "%c%c", STRAIGHT, switch_id_to_byte(node_id));
                    train_server->switches_status[node_id-1] = STRAIGHT;
                    Delay(20);
                    Putc(COM1, SOLENOID_OFF);
                    cli_update_switch(update_request->switch_update);
                    
                    // update switches UI
                    update_request->br_update[idx].id = node_id;
                    update_request->br_update[idx++].state = 's';
                }
            } else{
                if(train_server->switches_status[node_id-1] != CURVE){
                    // flip the switches 
                    dump(SUBMISSION, "%d sensor:%d\r\n", 0, node_id);
                    irq_printf(COM1, "%c%c", CURVE, switch_id_to_byte(node_id));
                    train_server->switches_status[node_id-1] = CURVE;
                    Delay(20);
                    Putc(COM1, SOLENOID_OFF);

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

// initialization 
/*track_node tracka[TRACK_MAX];*/
/*init_tracka(tracka);*/
