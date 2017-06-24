#include <calculation.h>

// TODO: deal with the case the input value is invalid 
int cal_distance(track_node *track, int src, int dest){
    /*bwprintf(COM2, "src = %d, dest=%d \r\n", src, dest);*/

    if(dest < 0 || src < 0 || dest > TRACK_MAX || src > TRACK_MAX){
        return 0;
    }

    fifo_t queue; 
    fifo_init(&queue);

    track[src].buf = 0; // initialize the distance 
    fifo_put(&queue, &(track[src]));

    while(!is_fifo_empty(&queue)){
        track_node *temp;
        fifo_get(&queue, &temp);

        if(temp->num == track[dest].num){
            return temp->buf;
        }

        if (temp->type == NODE_BRANCH){
            temp->edge[DIR_STRAIGHT].dest->buf = temp->buf + temp->edge[DIR_STRAIGHT].dist;
            fifo_put(&queue, temp->edge[DIR_STRAIGHT].dest);
            temp->edge[DIR_CURVED].dest->buf = temp->buf + temp->edge[DIR_CURVED].dist;
            fifo_put(&queue, temp->edge[DIR_CURVED].dest);
        } else{
            temp->edge[DIR_AHEAD].dest->buf = temp->buf + temp->edge[DIR_AHEAD].dist;
            fifo_put(&queue, temp->edge[DIR_AHEAD].dest);
        }
    }

    return -1; // error
}

// initialization 
    /*track_node tracka[TRACK_MAX];*/
    /*init_tracka(tracka);*/
