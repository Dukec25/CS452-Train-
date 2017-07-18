#include <calculation.h>
#include <log.h>
#include <debug.h>

int choose_destination(track_node *track, int src, int dest, Train_server *train_server){
    /*irq_debug(SUBMISSION, "src = %d, dest = %d", src, dest);*/
    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX || src == dest) {
        // value out of range, don't do anything
        debug(SUBMISSION, "invalid data src = %d, dest %d, in choose_destination", src, dest);
        return -1;
    }

    track_node *temp;
    temp = find_path(track, src, dest);
    /*irq_debug(SUBMISSION, "nothing wrong with %s\r\n", "find_path");*/
    bwprintf(COM2, "nothing wrong with %s\r\n", "find_path");
    return switches_need_changes(src, temp, train_server);
}

int cal_distance(track_node *track, int src, int dest)
{
    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX || src == dest) {
        debug(SUBMISSION, "invalid data src = %d, dest %d, in cal_distance", src, dest);
        return 0;
    }
	/*irq_debug(SUBMISSION, "%d %d", src, dest);*/
	/*dump(SUBMISSION, "%d %d", src, dest);*/
    track_node *temp;
    temp = find_path(track, src, dest);
    if (temp) {
        return temp->buf*1000;
    }
    else{
        return 0;
    }
}

// TODO: deal with the case the input value is invalid 
track_node* find_path(track_node *track, int src, int dest)
{
    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX || src == dest) {
        return NULL;
    }

    fifo_t queue; 
    fifo_init(&queue);

    track[src].buf = 0; // initialize the distance 
    fifo_put(&queue, &(track[src]));

    while (!is_fifo_empty(&queue)) {
        track_node *temp;
        fifo_get(&queue, &temp);

		/*dump(SUBMISSION, "visit %s", temp->name);*/

        if (strlen(temp->name) == strlen(track[dest].name)){
            if (!strcmp(temp->name, track[dest].name, strlen(temp->name))) {
                return temp;
            }
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

int switches_need_changes(int src, track_node *node, Train_server *train_server){
    /*debug(SUBMISSION, "switches_need_changes=%d\r\n", src);*/
    int idx = 0; // br_update size is 10

    track_node *temp = node;
    while(temp->num != src) {
        /*debug(SUBMISSION, "%s ", temp->name);*/
        temp = temp->previous;
    }
    /*debug(SUBMISSION, "%s \r\n", temp->name);*/

    while(node->num != src){
        /*debug(SUBMISSION, "visiting %s", node->name);*/
        if(node->previous->type != NODE_BRANCH){
            /*debug(SUBMISSION, "%s", "not branch");*/
            node = node->previous;
            continue;
        } else {
            /*debug(SUBMISSION, "%s", "branch");*/
            // the actual id of the current node
            int node_id = node->previous->num;
            switch(node_id){
                case 156:
                    node_id = 22;
                    break;
                case 155:
                    node_id = 21;
                    break;
                case 154:
                    node_id = 20;
                    break;
                case 153:
                    node_id = 19;
                    break;
                default:
                    break;
            }
            if(node->previous->edge[DIR_STRAIGHT].dest == node){
                /*debug(SUBMISSION, "straight %s\r\n", "");*/
                if(train_server->switches_status[node_id-1] != STRAIGHT){
                    /*debug(SUBMISSION, "%s", "need switch straight");*/
                    int merge_num = convert_sw_track_data(node_id, 1); // opposite of branch
                    int next_stop = predict_next(train_server->track, merge_num, train_server);
                    /*debug(SUBMISSION, "next stop %d", next_stop);*/

                    // revert the direction 
                    if(next_stop %2 == 0){
                        next_stop += 1;
                    } else{
                        next_stop -= 1;
                    }

                    /*irq_debug(SUBMISSION, "merge_num %d, current stop%d, previous sensor%d", merge_num, node_id, next_stop);   */

                    Train_br_switch br_switch;
                    br_switch.sensor_stop = next_stop;
                    br_switch.id  = node_id;
                    br_switch.state = 's';
                    push_br_lifo(train_server, br_switch);
                    idx++;

                } else{
                    /*debug(SUBMISSION, "status straight \r\n");*/
                }
            } else{
                /*debug(SUBMISSION, "curve %s\r\n", "");*/
                if(train_server->switches_status[node_id-1] != CURVE){
                    /*debug(SUBMISSION, "%s", "need switch curve");*/
                    /*debug(SUBMISSION, "current node %d", node_id);*/
                    int merge_num = convert_sw_track_data(node_id, 1);
                    /*debug(SUBMISSION, "reverse_num%d", merge_num);*/
                    int next_stop = predict_next(train_server->track, merge_num, train_server);
                    /*debug(SUBMISSION, "next stop%d", next_stop);*/

                    // revert the direction 
                    if(next_stop %2 == 0){
                        next_stop += 1;
                    } else{
                        next_stop -= 1;
                    }

                    /*irq_debug(SUBMISSION, "reverse_num %d, current stop%d, previous sensor%d", reverse_num, node_id, next_stop);   */

                    Train_br_switch br_switch;
                    br_switch.sensor_stop = next_stop;
                    br_switch.id  = node_id;
                    br_switch.state = 'c';
                    push_br_lifo(train_server, br_switch);
                    idx++;
                } else{
                    /*debug(SUBMISSION, "status curve \r\n");*/
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

    /*debug(SUBMISSION, "enter predict_next, src=%d, num=%d", src, temp->num);*/
    fifo_put(&queue, temp);

    while(1){
        fifo_get(&queue, &temp);
        if(temp->type == NODE_SENSOR){
            /*debug(SUBMISSION, "sensor node%d", temp->num);*/
            return temp->num;
        } else if(temp->type == NODE_BRANCH){
            /*debug(SUBMISSION, "branch node %d", temp->num);*/
            int cur_dir = train_server->switches_status[temp->num-1];
            if(cur_dir == STRAIGHT){
                fifo_put(&queue, temp->edge[DIR_STRAIGHT].dest);
            } else{
                fifo_put(&queue, temp->edge[DIR_CURVED].dest);
            }
        } else if(temp->type == NODE_MERGE){
            /*debug(SUBMISSION, "merge node%d", temp->num);*/
           fifo_put(&queue, temp->edge[DIR_AHEAD].dest); 
        } else if(temp->type == NODE_EXIT){
            return -1;
        }

    }
    return -1;
}

int find_stops_by_distance(track_node *track, int src, int dest, int stop_distance, Sensor_dist* ans){

    /*debug(SUBMISSION, "src=%d dest=%d dist=%d\r\n", src, dest, stop_distance);*/

    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX || src == dest) {
        // value out of range, don't do anything
        debug(SUBMISSION, "invalid data src = %d, dest %d, in find_stops_by_distance", src, dest);
        return -1;
    }

    track_node *node;
    node = find_path(track, src, dest);
	
	track_node *temp = node;
	while(temp->num != src) {
		/*debug(SUBMISSION, "%s ", temp->name);*/
    	temp = temp->previous;
	}
    /*debug(SUBMISSION, "%s \r\n", temp->name);*/

    fifo_t queue; 
    fifo_init(&queue);
    fifo_put(&queue, node);

    int arr_len = 0;
    int accumulated_distance = 0;
    
    while(1){
        track_node *cur_node;
        fifo_get(&queue, &cur_node);
        node = cur_node->previous;
		/*irq_debug(SUBMISSION, "visiting %s", node->name);*/

        if(node->type == NODE_BRANCH){
            if (strlen(node->edge[DIR_STRAIGHT].dest->name) != strlen(cur_node->name)){
				/*irq_debug(SUBMISSION, "decrement curve %d", node->edge[DIR_CURVED].dist);*/
                stop_distance -= node->edge[DIR_CURVED].dist;
                accumulated_distance += node->edge[DIR_CURVED].dist;
            } else{
                /*irq_debug(SUBMISSION, "decrement straight %s", node->edge[DIR_STRAIGHT].dest->name);*/
                stop_distance -= node->edge[DIR_STRAIGHT].dist; 
                accumulated_distance += node->edge[DIR_STRAIGHT].dist;
            }
        } else{
			/*irq_debug(SUBMISSION, "decrement ahead %d", node->edge[DIR_AHEAD].dist);*/
            stop_distance -= node->edge[DIR_AHEAD].dist;
            accumulated_distance += node->edge[DIR_AHEAD].dist;
        }

        fifo_put(&queue, node);

        if(node->type == NODE_SENSOR){
			/*irq_debug(SUBMISSION, "add %s", node->name);*/
            Sensor_dist sensor_dist;
            sensor_dist.sensor_id = node->num;
            sensor_dist.distance = accumulated_distance*1000;
            accumulated_distance = 0;
            ans[arr_len++]  = sensor_dist;

            if(stop_distance <=0){
                /*debug(SUBMISSION, "%s", "ENDED");*/
                return arr_len;
            }
        }

        if (strlen(node->name) == strlen(track[src].name)){
            if (!strcmp(node->name, track[src].name, strlen(node->name))) {
                // there is an error, probably too late to stop 
                return -1;
            }
        }
    }
}


