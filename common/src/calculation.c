#include <calculation.h>
#include <log.h>
#include <debug.h>
#include <train_server.h>

int cal_distance(track_node *track, int src, int dest)
{
    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX || src == dest) {
        debug(SUBMISSION, "invalid data src = %d, dest %d, in cal_distance", src, dest);
        return 0;
    }
	/*irq_debug(SUBMISSION, "%d %d", src, dest);*/
    track_node *temp;
    temp = find_path(track, src, dest);
    if (temp) {
        return temp->buf*1000;
    }
    else{
        return 0;
    }
}

int switches_need_changes(int src, track_node *node, Train_server *train_server, Br_lifo *br_lifo_struct){
    debug(SUBMISSION, "switches_need_changes=%d\r\n", src);
    int idx = 0; // br_update size is 10
    int pair_src = pair(src);

    track_node *temp = node;
    while(temp->num != src && temp->num != pair_src) {
        debug(SUBMISSION, "%s ", temp->name);
        temp = temp->previous;
    }
    debug(SUBMISSION, "%s", temp->name);

    while(node->num != src && node->num != pair_src){
        debug(SUBMISSION, "visiting %s\r\n", node->name);
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
            /*debug(SUBMISSION, "node_id%d", node_id);*/
            if(node->previous->edge[DIR_STRAIGHT].dest == node){
                /*debug(SUBMISSION, "straight %s\r\n", "");*/
                if(train_server->switches_status[node_id-1] != STRAIGHT){
                    int next_stop = previous_sensor_finder(node->previous);
                    debug(SUBMISSION, "%s", "switch to stright \r\n");

                    Train_br_switch br_switch;
                    br_switch.sensor_stop = next_stop;
                    br_switch.id  = node_id;
                    br_switch.state = 's';
                    push_br_lifo(br_lifo_struct, br_switch);

                } else{
                    /*debug(SUBMISSION, "status straight \r\n");*/
                }
            } else{
                /*debug(SUBMISSION, "curve %s\r\n", "");*/
                if(train_server->switches_status[node_id-1] != CURVE){
                    int next_stop = previous_sensor_finder(node->previous);
                    debug(SUBMISSION, "%s", "switch to curve \r\n");

                    Train_br_switch br_switch;
                    br_switch.sensor_stop = next_stop;
                    br_switch.id  = node_id;
                    br_switch.state = 'c';
                    push_br_lifo(br_lifo_struct, br_switch);
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
// path_finding that doesn't take into consideration of stop_direction
track_node* find_path_with_blocks(track_node *track, int src, int dest, int *resource)
{
    debug(SUBMISSION, "src = %d, dest = %d", src, dest);
    if (dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX || src == dest) {
        return NULL;
    }

    int pair_dest = 0;
    int pair_src = 0;

    pair_dest = pair(dest);
    pair_src  = pair(src)  ;
    debug(SUBMISSION, "pair_src = %d, pair_dest = %d", pair_src, pair_dest);

    fifo_t queue; 
    fifo_init(&queue);

    track[src].buf = 0; // initialize the distance 
    track[pair_src].buf = 0; // initialize the distance 

    fifo_put(&queue, &(track[src]));
    fifo_put(&queue, &(track[pair_src]));

    int visited_nodes[144];

    // used for internal counting visited nodes
    int i = 0;
    for(i = 0; i < 144; i++){
        visited_nodes[i] = 0;
    }

    while (!is_fifo_empty(&queue)) {
        track_node *temp;
        fifo_get(&queue, &temp);

        int node_num = get_track_idx(temp);
        /*debug(SUBMISSION, "current node %s, node_num %d", temp->name, node_num);*/
        if(node_num == -1){
            debug(SUBMISSION, "%s end", temp->name);
            continue; // this node is either type enter or exit 
        }

        if(visited_nodes[node_num]){
            debug(SUBMISSION, "%s already visited, node_num %d", temp->name, node_num);
            debug(SUBMISSION, "visited_node %d", visited_nodes[node_num]);
            continue;
        }

        visited_nodes[node_num] = 1;
        // edge case for beginning of the reverse route 
        if(node_num != src){
            visited_nodes[pair(node_num)] = 1;
        }

        if (resource[node_num] == 0 || resource[pair(node_num)] == 0){
            // the train not owns the sensor as a start location
            if(node_num != src && node_num != pair_src){
                debug(SUBMISSION, "Resource %s not available", temp->name);
                continue;
            }
        }

        // if destination is reached 
        if (strlen(temp->name) == strlen(track[dest].name) || 
            strlen(temp->name) == strlen(track[pair_dest].name) )
        {
            if (!strcmp(temp->name, track[dest].name, strlen(temp->name)) || 
                 !strcmp(temp->name, track[pair_dest].name, strlen(temp->name))  ) 
            {
                return temp;
            }
        }

        if (temp->type == NODE_BRANCH) {
            temp->edge[DIR_STRAIGHT].dest->buf = temp->buf + temp->edge[DIR_STRAIGHT].dist;
            temp->edge[DIR_STRAIGHT].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_STRAIGHT].dest);

            temp->edge[DIR_CURVED].dest->buf = temp->buf + temp->edge[DIR_CURVED].dist;
            temp->edge[DIR_CURVED].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_CURVED].dest);
        }
        else if(temp->type == NODE_MERGE){
            temp->reverse->buf = temp->buf;
            temp->reverse->previous = temp;

            temp->edge[DIR_AHEAD].dest->buf = temp->buf + temp->edge[DIR_AHEAD].dist;
            temp->edge[DIR_AHEAD].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_AHEAD].dest);

            if(temp->reverse->edge[DIR_STRAIGHT].dest != temp->previous){
                temp->reverse->edge[DIR_STRAIGHT].dest->buf = temp->buf +
                    temp->reverse->edge[DIR_STRAIGHT].dist;
                temp->reverse->edge[DIR_STRAIGHT].dest->previous =  temp->reverse;
                fifo_put(&queue, temp->reverse->edge[DIR_STRAIGHT].dest);
            } else{
                temp->reverse->edge[DIR_CURVED].dest->buf = temp->buf +
                    temp->reverse->edge[DIR_CURVED].dist;
                temp->reverse->edge[DIR_CURVED].dest->previous = temp->reverse;
                fifo_put(&queue, temp->reverse->edge[DIR_CURVED].dest);
            }
        }
        else {
            temp->edge[DIR_AHEAD].dest->buf = temp->buf + temp->edge[DIR_AHEAD].dist;
            temp->edge[DIR_AHEAD].dest->previous = temp;
            fifo_put(&queue, temp->edge[DIR_AHEAD].dest);
        }
    }

    debug(SUBMISSION, "%s", "no path available");
    return NULL;
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

// internal helper function for finding the previous within a process of a
// back tracking 
int previous_sensor_finder(track_node *node){
    track_node *temp = node;
    while(temp->type != NODE_SENSOR){
        temp = temp->previous;
    }
    return temp->num;
}

/*// to be deleted in the future*/
/*void push_br_lifo(Br_lifo *br_lifo_struct, Train_br_switch br_switch)*/
/*{*/
    /*if (br_lifo_struct->br_lifo_top != BR_LIFO_SIZE - 1) {*/
        /*br_lifo_struct->br_lifo_top += 1;*/
        /*br_lifo_struct->br_lifo[br_lifo_struct->br_lifo_top] = br_switch;*/
    /*}*/
/*}*/

/*void pop_br_lifo(Br_lifo *br_lifo_struct)*/
/*{*/
    /*if(br_lifo_struct->br_lifo_top == -1){*/
        /*// lifo is empty */
        /*return;*/
    /*}*/
    /*br_lifo_struct->br_lifo_top -= 1;*/
/*}*/

/*void push_track_cmd_fifo(Track_cmd_fifo_struct *track_cmd_fifo_struct, Track_cmd track_cmd)*/
/*{*/
    /*int track_cmd_fifo_put_next = track_cmd_fifo_struct->track_cmd_fifo_head + 1;*/
    /*if (track_cmd_fifo_put_next != track_cmd_fifo_struct->track_cmd_fifo_tail){*/
        /*if (track_cmd_fifo_put_next >= TRACK_CMD_FIFO_SIZE){*/
            /*track_cmd_fifo_put_next = 0;*/
        /*}*/
    /*}*/
    /*track_cmd_fifo_struct->track_cmd_fifo[track_cmd_fifo_struct->track_cmd_fifo_head] = track_cmd;*/
    /*track_cmd_fifo_struct->track_cmd_fifo_head = track_cmd_fifo_put_next;*/
/*}*/

/*void pop_track_cmd_fifo(Track_cmd_fifo_struct *track_cmd_fifo_struct, Track_cmd *track_cmd)*/
/*{*/
    /*int track_cmd_fifo_get_next = track_cmd_fifo_struct->track_cmd_fifo_tail + 1;*/
    /*if (track_cmd_fifo_get_next >= TRACK_CMD_FIFO_SIZE){*/
        /*track_cmd_fifo_get_next = 0;*/
    /*}*/
    /**track_cmd = track_cmd_fifo_struct->track_cmd_fifo[track_cmd_fifo_struct->track_cmd_fifo_tail];*/
    /*track_cmd_fifo_struct->track_cmd_fifo_tail = track_cmd_fifo_get_next;*/
/*}*/

int get_track_idx(track_node *temp){
    if(temp->type == NODE_ENTER || temp->type == NODE_EXIT){
        return -1; 
    }
    if(temp->type == NODE_SENSOR){
        return temp->num;
    }
    int node_id = temp->num;
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
    if(temp->type == NODE_BRANCH){
        return convert_sw_track_data(node_id, BRANCH);
    } else if(temp->type == NODE_MERGE){
        return convert_sw_track_data(node_id, MERGE);
    }
}

void put_cmd_fifo(track_node *track, int dest, int *resource, track_node *node, Train *train, 
        TS_request *ts_request){
    int src = train->last_stop;
    track_node *temp = node;

    int pair_src = pair(src);
    while(temp->num != src && temp->num != pair_src) {
        int node_id = get_track_idx(temp);
        // making all these segment unavailable 
        resource[node_id] = 0;
        resource[pair(node_id)] = 0;
        temp = temp->previous;
    }
    // include start loc
    resource[src] = 0;
    resource[pair_src] = 0;

    int reverse_at_start = 0;
    if(node->num != dest){
        // indicate direction reversed
        // actual dest is 5, the real dest is 40 
        debug(SUBMISSION, "actual dest is %d, the real dest is %d", dest, node->num);
        reverse_at_start = 1; 
    }

    Lifo_t parsing_table;
    lifo_init(&parsing_table);
    lifo_push(&parsing_table, node); // push in the dest
    
    fifo_t queue; 
    fifo_init(&queue);
    fifo_put(&queue, node->previous); // start with second last

    // constructing parsing table 
    while(1){
        track_node *cur_node;
        fifo_get(&queue, &cur_node);
        irq_debug(SUBMISSION, "visiting %s", cur_node->name);

        // make sure previous is the reverse of the branch
        if(cur_node->type == NODE_BRANCH){
            int current_num = get_track_idx(cur_node);
            int previous_num = get_track_idx(cur_node->previous);
            /*irq_debug(SUBMISSION, "current_num %d, previous_num %d", current_num, previous_num);*/
            if(current_num == pair(previous_num)){
                irq_debug(SUBMISSION, "name is %s", cur_node->previous->name);
                lifo_push(&parsing_table , cur_node->previous);
            }
        }

        fifo_put(&queue, cur_node->previous);

        if (strlen(cur_node->name) == strlen(track[src].name)){
            if (!strcmp(cur_node->name, track[src].name, strlen(cur_node->name))) {
                debug(SUBMISSION, "name is %s", cur_node->name);
                lifo_push(&parsing_table, cur_node);
                break;
            }
        }
        if (strlen(cur_node->name) == strlen(track[pair_src].name)){
            if (!strcmp(cur_node->name, track[pair_src].name, strlen(cur_node->name))) {
                debug(SUBMISSION, "name is %s", cur_node->name);
                lifo_push(&parsing_table, cur_node);
                break;
            }
        }
    }
    generate_cmds_table(track, &parsing_table, reverse_at_start, train, ts_request);
}

void generate_cmds_table(track_node *track, Lifo_t *parsing_table, int reverse, Train *train, 
        TS_request *ts_request){
    Track_cmd_fifo_struct result; 
    result.track_cmd_fifo_head = 0;
    result.track_cmd_fifo_tail = 0;

    if(reverse){
        debug(SUBMISSION, "%s", "reverse at beginning");
        Track_cmd track_cmd;
        track_cmd.type = TRACK_REVERSE;
        push_track_cmd_fifo(&result, track_cmd);
    }
    
    track_node *previous_node; // first node 
    lifo_pop(parsing_table, &previous_node);
    debug(SUBMISSION, "first name is %s", previous_node->name);
    while(!is_lifo_empty(parsing_table)){
        track_node *cur_node;
        lifo_pop(parsing_table, &cur_node);
        debug(SUBMISSION, "name is %s", cur_node->name);
        int previous_num = get_track_idx(previous_node);
        int current_num = get_track_idx(cur_node);
        if(cur_node->type == NODE_MERGE){
            // distance to the node that before the switches
            int node_before_switch_num = get_track_idx(cur_node->previous);
            if(previous_num != node_before_switch_num){
                int distance = cal_distance(track, previous_num, node_before_switch_num);
                debug(SUBMISSION, "merge distance is %d", distance);
                if(distance > 90*10000){ //TODO, 90cm should be modified in the future
                    debug(SUBMISSION, "%s", "park operation");
                    Track_cmd track_cmd_park;
                    track_cmd_park.type = TRACK_PARK;
                    calculate_park(cur_node, train, &track_cmd_park.park_info);
                    push_track_cmd_fifo(&result, track_cmd_park);
                } else{
                    Track_cmd track_cmd_slow;
                    track_cmd_slow.type = TRACK_SLOW_WALK;
                    track_cmd_slow.distance = distance; 
                    push_track_cmd_fifo(&result, track_cmd_slow);
                }
            }
            //currently perform slow walks from previous node to merge node
            Track_cmd track_cmd_slow;
            track_cmd_slow.type = TRACK_SLOW_WALK;
            // distance between switches and previous node 
            track_cmd_slow.distance = cal_distance(track, node_before_switch_num, current_num) +
                TRAIN_LENGTH; 
            debug(SUBMISSION, "merge distance is %d", track_cmd_slow.distance);
            push_track_cmd_fifo(&result, track_cmd_slow);

            Track_cmd track_cmd_reverse;
            track_cmd_reverse.type = TRACK_REVERSE;
            push_track_cmd_fifo(&result, track_cmd_reverse);

            previous_node = cur_node->reverse; 
        } else if(cur_node->type == NODE_SENSOR){
            // the dest node 
            int distance = cal_distance(track, previous_num, current_num);
            if(distance > 90*10000){ // 90 cm should be modified in future
                debug(SUBMISSION, "%s", "park operation");
                Track_cmd track_cmd_park;
                track_cmd_park.type = TRACK_PARK;
                calculate_park(cur_node, train, &track_cmd_park.park_info);
                push_track_cmd_fifo(&result, track_cmd_park);
            } else{
                Track_cmd track_cmd_slow;
                track_cmd_slow.type = TRACK_SLOW_WALK;
                track_cmd_slow.distance = distance; 
                push_track_cmd_fifo(&result, track_cmd_slow);
            }
            debug(SUBMISSION, "final destination %s", "reached");
            break;
        } else {
            debug(SUBMISSION, "sth is wrong, type is %d", cur_node->type);
        }
    }
    ts_request->track_result.cmd_fifo_struct = result;
}

void calculate_park(track_node *node, Train *train, Park_info *park_info){
    fifo_t queue; 
    fifo_init(&queue);
    fifo_put(&queue, node);
    int arr_len = 0;
    int accumulated_distance = 0;
    int stop_distance = train->velocity_model.stopping_distance[GO_CMD_FINAL_SPEED];
    Sensor_dist park_stops[SENSOR_GROUPS * SENSORS_PER_GROUP];

    /*debug(SUBMISSION, "before %s", "while");*/
    
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
            park_stops[arr_len++]  = sensor_dist;
            if(stop_distance <=0){
                /*debug(SUBMISSION, "%s", "ENDED");*/
                break;
            }
        }
        // too late for a train to stop should never happen
        // trains should be able to get to anywhere 
    }
    int num_park_stops = arr_len;
    int deaccelarate_stop = park_stops[num_park_stops - 1].sensor_id; // need to fill in
    // calculate the delta = the distance between sensor_to_deaccelate_train
    // calculate average velocity measured in [tick]
    int delta = 0;
    /*int velocity = train->velocity_model.velocity[train->speed];*/

    int i = 0;

    for ( i = 0; i < num_park_stops; i++) {
        delta += park_stops[i].distance;
    }
    int stopping_distance = train->velocity_model.stopping_distance[train->speed];
    /*int park_delay_time = (delta - stopping_distance * 1000) / velocity;*/
    park_info->delay_distance = delta - stopping_distance * 1000;
    park_info->deaccel_stop = deaccelarate_stop;
}

void manage_resource(int sensor_num, int *resource, Train_server *train_server){
    //TODO pay attention for bugs here 
    int pair_num = pair(sensor_num);
    resource[sensor_num] = 1;
    resource[pair_num] = 1;

    track_node *node = &train_server->track[sensor_num];
    // free the previous node 
    int previous_num = previous_sensor_finder(node);
    int previous_pair_num = pair(pair_num);
    resource[previous_num] = 0;
    resource[previous_pair_num] = 0;
}
