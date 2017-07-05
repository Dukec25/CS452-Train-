#ifndef __NAME_SERVER_H__
#define __NAME_SERVER_H__
#include <kernel.h>

#define MAX_NUM_NAMES	2

typedef enum Server_err {
	SERVER_ERR_SUCCESS					= 0x0,
	SERVER_ERR_FAILURE					= -1,
	SERVER_ERR_ALREADY_REGISTERED		= 0x80,
	SERVER_ERR_NOT_REGISTERED			= 0x81,
	SERVER_ERR_MAX_NUM_NAMES_REACHED	= 0x82
} Server_err;

typedef enum Message_type {
	MSG_SUCCESS,
	MSG_FAILURE,
	MSG_REGITSER_AS,
	MSG_WHO_IS
} Message_type;

typedef struct Name_server_message {
	int tid;
	Message_type type;
	char content[MAX_MSG_LEN];
} Name_server_message;

typedef struct Name_server {
	int tid;
	int tid_filled[MAX_NUM_TASKS];	// keep track of the number of names associated with the tid
	int req_map_pos;
	Name_server_message req_map[MAX_NUM_TASKS * MAX_NUM_NAMES];	// use tid or tid + 1 to look up the name
} Name_server;

void name_server_start();
int RegisterAs(char *name);
int WhoIs(char *name);
#endif // __NAME_SERVER_H__
