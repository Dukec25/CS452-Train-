#ifndef __TRACK_SERVER__
#define __TRACK_SERVER__

#include <train.h>


typedef enum {
    BRANCH,
    MERGE
} branch_type;

void park_server();
int pair(int idx);
// type = 0 branch, 1 merge
void track_server();
int convert_sw_track_data(int num, int type);
int choose_rand_destination();

#endif // __TRACK_SERVER__

