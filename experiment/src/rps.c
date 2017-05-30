#include <rps.h>
#include <kernel.h>
#include <user_functions.h>
#include <name_server.h>
#include <bwio.h>

static const char RPS_SERVER_NAME[] = "rps_server";
static uint32 choice_seed = 0;

void rps_server_initialize(RPS_server *rps_server)
{
	rps_server->tid = MyTid();
	rps_server->is_playing = 0;
	debug(DEBUG_TASK, "is_playing = %d", rps_server->is_playing);
	rps_server->is_quit = 0;
	rps_server->is_server_running = 1;

	rps_server->num_players = 0;
	rps_server->player1_tid = INVALID_TID;
	rps_server->player1_choice = INVALID_CHOICE;
	rps_server->player2_tid = INVALID_TID;
	rps_server->player2_choice = INVALID_CHOICE;
	fifo_init(&(rps_server->player_queue));

	int i = 0;
	for (i = 0; i < MAX_NUM_TASKS; i++) {
		rps_server->signed_in_list[i] = 0;
	}
}

void rps_server_start()
{
	debug(DEBUG_TASK, "enter %s", "rps_server_start");
	RPS_server rps_server;
	rps_server_initialize(&rps_server);

	int register_result = RegisterAs("RPS_SERVER_NAME");
	debug(DEBUG_TASK, "RegisterAs result = %d", register_result);

	if (register_result) {
		debug(KERNEL2, "RPS server RegisterAs failed, error code = %d, Exiting %d", register_result, rps_server.tid);
		Exit();
	}
	debug(KERNEL2, "Successfully registered rps server as %s", "rps_server");

	int continue_pressed = 0;
	while(rps_server.is_server_running) {
		RPS_message request;
		int tid;
		Receive(&tid, &request, sizeof(request));
		switch(request.type) {
			case RPS_MSG_SIGN_IN:
			debug(DEBUG_TASK, "%s %d", "RPS_MSG_SIGN_IN", tid);
			rps_handle_sign_in(&rps_server, &request);
			break;
			case RPS_MSG_PLAY:
			debug(DEBUG_TASK, "%s %d", "RPS_MSG_PLAY", tid);
			rps_handle_play(&rps_server, &request);
			break;
			case RPS_MSG_QUIT:
			debug(DEBUG_TASK, "%s %d", "RPS_MSG_QUIT", tid);
			rps_handle_quit(&rps_server, &request);
			break;
			default:
			debug(DEBUG_TASK, "%s %d", "WARNING!!!!!!!Unkown", tid);
			break;
		}

		Pass();

		if (is_fifo_empty(&(rps_server.player_queue)) && !(rps_server.is_playing) && (rps_server.num_players == 0)) {
			// No player in queue
			debug(DEBUG_TASK, "No player in queue, and player is playing, %s", "stop");
			if (!continue_pressed) {
				debug(KERNEL2, "%s", "End one game, press 'q' to quit, ANY other key to continue");
				char c = bwgetc(COM2);
				if (c == 'q') {
					rps_server.is_server_running = 0;
				}
				continue_pressed = 1;
			}
			else {
				continue_pressed = 0;
			}
		}
	}
	debug(DEBUG_TASK, "Exiting %d", rps_server.tid); 
	Exit();
}

void rps_reply_server_down(RPS_server *rps_server, int tid) {
	debug(DEBUG_TASK, "enter %s", "rps_reply_server_down");
	RPS_message reply;
	reply.tid = tid;
	reply.type = RPS_MSG_SERVER_DOWN;
	reply.content[0] = '\0';
	Reply(reply.tid, &reply, sizeof(reply));
}

void rps_handle_sign_in(RPS_server *rps_server, RPS_message *req)
{
	debug(DEBUG_TASK, "enter %s", "rps_handle_sign_in");

	if (!rps_server->is_server_running) {
		rps_reply_server_down(rps_server, req->tid);
		return;
	}

	if (is_fifo_full(&(rps_server->player_queue))) {
		// player queue is full, reply sign in failure message
		RPS_message reply;
		reply.tid = req->tid;
		reply.type = RPS_MSG_FAILURE;
		reply.content[0] = '\0';
		Reply(reply.tid, &reply, sizeof(reply));
	}
	else {
		// put player to the end of player queue, and update signed_in_list 
		fifo_put(&(rps_server->player_queue), req);
		rps_server->signed_in_list[req->tid] = 1;
		debug(DEBUG_TASK, "put %d into the end of player_queue, signed_in_list[%d] = %d",
							req->tid, req->tid, rps_server->signed_in_list[req->tid]);
		rps_pair_players(rps_server);
		// reply sign in successfull message
		RPS_message reply;
		reply.tid = req->tid;
		reply.type = RPS_MSG_SUCCESS;
		reply.content[0] = '\0';
		Reply(reply.tid, &reply, sizeof(reply));
	}
}

void rps_pair_players(RPS_server *rps_server)
{
	debug(DEBUG_TASK, "enter %s", "rps_pair_players");
	
	if (rps_server->is_playing) {
		debug(DEBUG_TASK, "is_playing = %d, return", rps_server->is_playing);
		return;
	}
	if(fifo_get_count(&(rps_server->player_queue)) < 2) {
		debug(DEBUG_TASK, "not enough players %d, return", fifo_get_count(&(rps_server->player_queue)));
		return;
	}

	int players[2];
	players[0] = INVALID_TID;
	players[1] = INVALID_TID;
	int count = 0;
	while(!is_fifo_empty(&(rps_server->player_queue)) && (rps_server->num_players < 2)) {
		RPS_message *req;
		fifo_get(&(rps_server->player_queue), &req);
		debug(DEBUG_TASK, "get req %d, signed_in_list[tid] = %d, count = %d", req->tid, rps_server->signed_in_list[req->tid], count);
		if (rps_server->signed_in_list[req->tid]) {
			players[count] = req->tid;
		}
		else {
			players[count] = INVALID_TID;
		}
		rps_server->num_players++;
		count++;
	}

	if (players[0] == players[1]) {
		// Cannot play with itself
		rps_server->num_players = 1;
		debug(DEBUG_TASK, "Cannot play with itself %d, num_players = %d", players[0], rps_server->num_players);
		return;
	}
	
	if (rps_server->num_players == 2) {
		rps_server->is_playing = 1;
	}
	if (count == 2) {
		// Match players
		debug(DEBUG_TASK, "!!!Got a pair of new players %d, %d", players[0], players[1]);
		rps_server->player1_tid = players[0];
		rps_server->player2_tid = players[1];
		debug(DEBUG_TASK, "player1_tid = %d, player2_tid = %d, is_playing = %d",
							rps_server->player1_tid, rps_server->player2_tid, rps_server->is_playing);
	}
	else if (count == 1) {
		// Match players
		debug(DEBUG_TASK, "!!!Got one new player %d", players[0]);
		if (rps_server->num_players == 2) {
			rps_server->player2_tid = players[0];
		} else {
			rps_server->player1_tid = players[0];
		}
		debug(DEBUG_TASK, "player1_tid = %d, player2_tid = %d, is_playing = %d",
							rps_server->player1_tid, rps_server->player2_tid, rps_server->is_playing);
	}
}

void rps_handle_play(RPS_server *rps_server, RPS_message *req)
{
	debug(DEBUG_TASK, "enter %s", "rps_handle_play");
	if (!rps_server->is_server_running) {
		rps_reply_server_down(rps_server, req->tid);
		return;
	}

	// update player1 and player2 choice
	int player_tid = req->tid;
	RPS_choice player_choice = req->content[0];
	if (player_tid == rps_server->player1_tid) {
		rps_server->player1_choice = player_choice;
		debug(DEBUG_TASK, "update player1_choice = %d", rps_server->player1_choice);
	}
	else if (player_tid == rps_server->player2_tid) {
		rps_server->player2_choice = player_choice;
		debug(DEBUG_TASK, "update player2_choice = %d", rps_server->player2_choice);
	}

	if (!rps_server->is_playing) {
		// rps server is not ready to play game yet, in the matching pair phase
		debug(DEBUG_TASK, "is_playing = %d, not ready to play yet", rps_server->is_playing);
		return;
	}

	if (rps_server->num_players == 2 && 
		rps_server->player1_choice != INVALID_CHOICE && 
		rps_server->player2_choice != INVALID_CHOICE) {
		debug(DEBUG_TASK, "both player %d and player %d have given their choices %d %d, start to reply result",
							rps_server->player1_tid, rps_server->player2_tid, rps_server->player1_choice, rps_server->player2_choice);
		rps_reply_result(rps_server);
		// reset players choice
		rps_server->player1_choice = INVALID_CHOICE;
		rps_server->player2_choice = INVALID_CHOICE;
	}
}

void rps_handle_quit(RPS_server *rps_server, RPS_message *req)
{
	debug(DEBUG_TASK, "enter %s", "rps_handle_quit");
	if (!rps_server->is_server_running) {
		rps_reply_server_down(rps_server, req->tid);
		return;
	}

	// iniatialize reply message
	RPS_message reply;
	reply.tid = req->tid;
	reply.content[0] = '\0';
	reply.type = RPS_MSG_GOODBYE;

	// reset is_playing, num_players, and players
	rps_server->is_playing = 0;
	rps_server->num_players--;
	
	rps_server->player1_tid = INVALID_TID;
	rps_server->player1_choice = INVALID_CHOICE;
	rps_server->player2_tid = INVALID_TID;
	rps_server->player2_choice = INVALID_CHOICE;
	debug(DEBUG_TASK, "is_playing = %d", rps_server->is_playing);

	// unset corresponding slot in signed_in_list
	rps_server->signed_in_list[rps_server->player1_tid] = 0;
	rps_server->signed_in_list[rps_server->player2_tid] = 0;
	debug(DEBUG_TASK, "signed_in_list[%d] = %d, signed_in_list[%d] = %d",
					rps_server->player1_tid, rps_server->signed_in_list[rps_server->player1_tid],
					rps_server->player2_tid, rps_server->signed_in_list[rps_server->player2_tid]);

	Reply(reply.tid, &reply, sizeof(reply));
}

void rps_reply_result(RPS_server *rps_server)
{
	debug(DEBUG_TASK, "enter %s", "rps_reply_result");
	if (!rps_server->is_server_running) {
		rps_reply_server_down(rps_server, rps_server->player1_tid);
		rps_reply_server_down(rps_server, rps_server->player2_tid);
		return;
	}

	RPS_outcome outcome1 = INVALID_OUTCOME;
	RPS_outcome outcome2 = INVALID_OUTCOME;
	if (rps_server->player1_choice == rps_server->player2_choice) {
		outcome1 = TIE;
		outcome2 = TIE;
	}
	else if (rps_server->player1_choice == ROCK && rps_server->player2_choice == PAPER) {
		outcome1 = LOSE;
		outcome2 = WIN;
	}
	else if (rps_server->player1_choice == ROCK && rps_server->player2_choice == SCISSOR) {
		outcome1 = WIN;
		outcome2 = LOSE;
	}
	else if (rps_server->player1_choice == PAPER && rps_server->player2_choice == ROCK) {
		outcome1 = WIN;
		outcome2 = LOSE;
	}
	else if (rps_server->player1_choice == PAPER && rps_server->player2_choice == SCISSOR) {
		outcome1 = LOSE;
		outcome2 = WIN;
	}
	else if (rps_server->player1_choice == SCISSOR && rps_server->player2_choice == ROCK) {
		outcome1 = LOSE;
		outcome2 = WIN;
	}
	else if (rps_server->player1_choice == SCISSOR && rps_server->player2_choice == PAPER) {
		outcome1 = WIN;
		outcome2 = LOSE;
	}

	debug(DEBUG_TASK, "player %d vs player %d: %d vs %d -> outcome1 = %d, outcome2 = %d",
						rps_server->player1_tid, rps_server->player2_tid,
						rps_server->player1_choice, rps_server->player2_choice, outcome1, outcome2);
	RPS_message reply1;
	reply1.tid = rps_server->player1_tid;
	reply1.type = RPS_MSG_OUTCOME;
	reply1.content[0] = outcome1;
	reply1.content[1] = '\0';
	Reply(reply1.tid, &reply1, sizeof(reply1));
	RPS_message reply2;
	reply2.tid = rps_server->player2_tid;
	reply2.type = RPS_MSG_OUTCOME;
	reply2.content[0] = outcome2;
	reply2.content[1] = '\0';
	Reply(reply2.tid, &reply2, sizeof(reply1));
}

uint32 rand(uint32 state[static 1])
{
	uint32 x = state[0];
	x ^= x << 13;
	x ^= x >> 17;
	x ^= x << 5;
	state[0] = x;
	return x >= x ? x : -x;
}

void rps_client_initialize(RPS_client *rps_client)
{
	rps_client->tid = MyTid();
}

void rps_client_start()
{
	debug(DEBUG_TASK, "enter %s", "rps_client_start");
	RPS_client rps_client;
	rps_client_initialize(&rps_client);
	debug(DEBUG_TASK, "player %d", rps_client.tid);

	int result = 0;

	int server_tid = WhoIs("RPS_SERVER_NAME");
	debug(DEBUG_TASK, "server_tid = %d", server_tid);	
	if (server_tid >= MAX_NUM_TASKS) {
		debug(KERNEL2, "WhoIs failed, invalid server tid %d", server_tid);
	}

	result = rps_client_sign_in(server_tid, &rps_client);
	if (result == -1) {
		debug(KERNEL2, "player %d failed to sign in, Exiting", rps_client.tid);
		Exit();
	}

	int i = 0;
	for (i = 0; i < NUM_ROUNDS; i++) {
		result = rps_client_play(server_tid, &rps_client, i);
		if (result == -1) {
			debug(KERNEL2, "player %d failed to play", rps_client.tid);
		}
	}

	result = rps_client_quit(server_tid, &rps_client);
	if (result == -1) {
		debug(KERNEL2, "player %d failed to quit, Exiting", rps_client.tid);
		Exit();
	}
	Exit();
}

int rps_client_sign_in(int server_tid, RPS_client *rps_client)
{
	debug(DEBUG_TASK, "enter %s", "rps_client_sign_in");
	RPS_message sign_in_request;
	RPS_message *request = &sign_in_request;
	request->tid = rps_client->tid;
	request->content[0] = '\0';
	request->type = RPS_MSG_SIGN_IN;
	RPS_message reply;

	Send(server_tid, request, sizeof(request), &reply, sizeof(reply));
	
	if (reply.type != RPS_MSG_SUCCESS) {
		return -1;
	}
	debug(KERNEL2, "player %d successfully signed in", rps_client->tid);
	return 0;
}

int rps_client_play(int server_tid, RPS_client *rps_client, int round)
{
	debug(DEBUG_TASK, "enter %s", "rps_client_play");
	RPS_message play_request;
	RPS_message *request = &play_request;
	request->tid = rps_client->tid;
	RPS_choice choice = rand(&choice_seed) % 3;
	request->content[0] = choice;
	request->content[1] = '\0';
	request->type = RPS_MSG_PLAY;
	RPS_message reply;
	debug(DEBUG_TASK, "player %d is choose to play %d", rps_client->tid, request->content[0]);

	Send(server_tid, request, sizeof(request), &reply, sizeof(reply));
	
	if (reply.type == RPS_MSG_SERVER_DOWN) {
		debug(KERNEL2, "server is down, Exiting", rps_client->tid);
		Exit();
	}
	if (choice == ROCK) {
		debug(KERNEL2, "player %d choose ROCK for round #%d", rps_client->tid, round + 1);
	}
	else if (choice == PAPER) {
		debug(KERNEL2, "player %d choose PAPER for round #%d", rps_client->tid, round + 1);
	}
	else if (choice == SCISSOR) {
		debug(KERNEL2, "player %d choose SCISSOR for round #%d", rps_client->tid, round + 1);
	}

	if (reply.type == RPS_MSG_OUTCOME) {
		debug(KERNEL2, "press any KEY to display round #%d result for player %d", round + 1, rps_client->tid);
		char c = bwgetc(COM2);
		RPS_outcome outcome = reply.content[0];
		if (outcome == WIN) {
			debug(KERNEL2, "result of this round #%d for player %d is WIN", round + 1, rps_client->tid);
		}
		else if (outcome == TIE) {
			debug(KERNEL2, "result of this round #%d for player %d is TIE", round + 1, rps_client->tid);
		}
		else if (outcome == LOSE) {
			debug(KERNEL2, "result of this round #%d for player %d is LOSE", round + 1, rps_client->tid);
		}
		return 0;
	}

	return -1;
}

int rps_client_quit(int server_tid, RPS_client *rps_client)
{
	debug(DEBUG_TASK, "enter %s", "rps_client_quit");
	RPS_message quit_request;
	RPS_message *request = &quit_request;
	request->tid = rps_client->tid;
	request->content[0] = '\0';
	request->type = RPS_MSG_QUIT;
	RPS_message reply;

	Send(server_tid, request, sizeof(request), &reply, sizeof(reply));
	
	if (reply.type == RPS_MSG_SERVER_DOWN) {
		debug(KERNEL2, "server is down, Exiting %d", rps_client->tid);
		Exit();
	}
	else if (reply.type == RPS_MSG_GOODBYE) {
		debug(KERNEL2, "Goodbye player %d", rps_client->tid);
		Exit();
	}

	return -1;
}
