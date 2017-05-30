#ifndef __RPS_H__
#define __RPS_H__

#include <fifo.h>
#include <debug.h>
#include <kernel.h>

#define NUM_ROUNDS	2

typedef enum RPS_choice {
	ROCK,
	PAPER,
	SCISSOR,
	INVALID_CHOICE = -1
} RPS_choice;

typedef enum RPS_outcome {
	LOSE,
	TIE,
	WIN,
	INVALID_OUTCOME
} RPS_outcome;

typedef enum RPS_message_type {
	// server
	RPS_MSG_SUCCESS,
	RPS_MSG_FAILURE,
	RPS_MSG_SERVER_DOWN,
	RPS_MSG_OUTCOME,
	RPS_MSG_GOODBYE,
	RPS_MSG_GAME_OVER,
	// client
	RPS_MSG_SIGN_IN,
	RPS_MSG_PLAY,
	RPS_MSG_QUIT
} RPS_message_type;

typedef struct RPS_message {
	int tid;
	RPS_message_type type;
	char content[MAX_MSG_LEN];
} RPS_message;

typedef struct RPS_server {
	int tid;
	int is_playing;	// both players are playing
	int is_quit;	// one of the player has quit
	int is_server_running;

	// active players
	int num_players;
	int player1_tid;
	RPS_choice player1_choice;
	int player2_tid;
	RPS_choice player2_choice;
	fifo_t player_queue;	

	// keep track of which player has signed in, which has quit
	uint8 signed_in_list[MAX_NUM_TASKS];	
} RPS_server;

typedef struct RPS_client {
	int tid;
} RPS_client;

// server
void rps_server_initialize(RPS_server *rps_server);
void rps_server_start();
void rps_handle_sign_in(RPS_server *rps_server, RPS_message *req);
void rps_handle_play(RPS_server *rps_server, RPS_message *req);
void rps_handle_quit(RPS_server *rps_server, RPS_message *req);
void rps_pair_players(RPS_server *rps_server);
void rps_reply_result(RPS_server *rps_server);

// random number generator
uint32 rand(uint32 state[static 1]);

// client
void rps_client_initialize();
void rps_client_start();
int rps_client_sign_in(int sever_tid, RPS_client *rps_client);
int rps_client_play(int sever_tid, RPS_client *rps_client, int round);
int rps_client_quit(int sever_tid, RPS_client *rps_client);

#endif // __RPS_H__
