#include <calculation.h>

void choose_destination(track_node *track, int src, int dest, Train_server *train_server){
    track_node *temp;
    temp = find_path(track, src, dest);
    switches_need_changes(src, temp, train_server);
}

int cal_distance(track_node *track, int src, int dest){
    track_node *temp;
    temp = find_path(track, src, dest);
    if(temp){
        return temp->buf;
    }
    else{
        return 0;
    }
}

// TODO: deal with the case the input value is invalid 
track_node* find_path(track_node *track, int src, int dest){
    bwprintf(COM2, "src = %d, dest=%d \r\n", src, dest);

    if(dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX){
        return NULL;
    }

    fifo_t queue; 
    fifo_init(&queue);

    track[src].buf = 0; // initialize the distance 
    fifo_put(&queue, &(track[src]));

    while(!is_fifo_empty(&queue)){
        track_node *temp;
        fifo_get(&queue, &temp);

        if(temp->num == track[dest].num){
            return temp;
        }

        if(temp->type == NODE_EXIT){
            continue; 
        }
        else if (temp->type == NODE_BRANCH){
            temp->edge[DIR_STRAIGHT].dest->buf = temp->buf + temp->edge[DIR_STRAIGHT].dist;
            temp->edge[DIR_STRAIGHT].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_STRAIGHT].dest);

            temp->edge[DIR_CURVED].dest->buf = temp->buf + temp->edge[DIR_CURVED].dist;
            temp->edge[DIR_CURVED].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_CURVED].dest);
        } else{
            temp->edge[DIR_AHEAD].dest->buf = temp->buf + temp->edge[DIR_AHEAD].dist;
            temp->edge[DIR_AHEAD].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_AHEAD].dest);
        }
    }

    return NULL;
}

void switches_need_changes(int src, track_node *node, Train_server *train_server){
    while(node->num != src){
        if(node->previous->type != NODE_BRANCH){
            node = node->previous;
            continue;
        } else {
            int sensor_id = node->previous->num;
            if(node->previous->edge[DIR_STRAIGHT].dest == node){
                if(train_server->switches_status[sensor_id-1] != STRAIGHT){
                    // flip the switches 
                    irq_printf(COM1, "%c%c", switch_state_to_byte(STRAIGHT), switch_id_to_byte(sensor_id));
                    Delay(20);
                    Putc(COM1, SOLENOID_OFF);
                }
            } else{
                if(train_server->switches_status[sensor_id-1] != CURVE){
                    // flip the switches 
                    irq_printf(COM1, "%c%c", switch_state_to_byte(CURVE), switch_id_to_byte(sensor_id));
                    Delay(20);
                    Putc(COM1, SOLENOID_OFF);
                }
            }
            node = node->previous;
        }
    }
}

// initialization 
/*track_node tracka[TRACK_MAX];*/
/*init_tracka(tracka);*/
