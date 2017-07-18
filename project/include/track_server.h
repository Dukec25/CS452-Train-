#ifndef __TRACK_SERVER__
#define __TRACK_SERVER__

#include <train.h>
#include <train_task.h>

typedef enum {
    TRAIN_WANT_GUIDANCE, // select random destination and route 
    TRAIN_WANT_RESULT
} Track_request_type;

typedef struct Track_request{
    Train *train;
	Track_request_type type;
} Track_request;

typedef struct Track_server {
    // 1 represent resource available, 0 otherwise 
    int resource_available[143]; 

#define ROUTE_RESULT_FIFO_SIZE	50
    TS_request route_result_fifo[TS_RESULT_FIFO_SIZE];
	int route_result_fifo_head;
	int route_result_fifo_tail;
} Track_server;

typedef enum {
    BRANCH,
    MERGE
} branch_type;
void park_server();
int pair(int idx);
// type = 0 branch, 1 merge
int convert_sw_track_data(int num, int type);
void push_ts_req_fifo(Track_server *track_server, TS_request ts_req);
void pop_ts_req_fifo(Track_server *train_server, TS_request *ts_req);
void br_handle(Train_server *train_server, Command br_cmd);

#endif // __TRACK_SERVER__

