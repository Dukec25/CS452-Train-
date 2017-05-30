#include <name_server.h>
#include <user_functions.h>
#include <string.h>

static int NAME_SERVER_TID = INVALID_TID;

#define	WRITE_ONCE_NAME_SERVER_TID(tid)													\
		do {																			\
				NAME_SERVER_TID = tid;													\
				debug(DEBUG_ALL, "!!!update NAME_SERVER_TID = %d", NAME_SERVER_TID);	\
			} while(0)	

#define READ_NAME_SERVER_TID(pval)		\
		do {							\
			*pval = NAME_SERVER_TID;	\
		} while(0)
 
static void initialize(Name_server *ns)
{
	debug(DEBUG_SYSCALL, "enter %s", "initialize");
	ns->tid = MyTid();
	WRITE_ONCE_NAME_SERVER_TID(ns->tid);
	debug(DEBUG_SYSCALL, "NAME_SERVER_TID = %d", NAME_SERVER_TID);
	int i = 0;
	for (i = 0; i < MAX_NUM_TASKS; i++) {
		// initalize tid_filled
		ns->tid_filled[i] = 0;

		// initalize req_map
		ns->req_map[i].tid = INVALID_TID;
		//ns->req_map[i].content[0] = '\0';
		ns->req_map[i + 1].tid = INVALID_TID;
		//ns->req_map[i + 1].content[0] = '\0';	
	}
	ns->req_map_pos = 0;
}

/*
 * Given a name, return SERVER_ERR_SUCCESS if name is in the req_map.
 * Otherwise, return SERVER_ERR_NOT_REGISTERED.
 * postion is update as the location of the req in the map if exist.
 */
static Server_err locate_service(Name_server *ns, char *name, int *postion)
{
	int is_found = 0;
	int idx = 0;
	for(idx = 0; idx < ns->req_map_pos; idx++) {
		debug(DEBUG_SYSCALL, "!!!!search for %s len %d, current idx = %d, service = %s len %d",
								name, strlen(name), idx, ns->req_map[idx].content, strlen(ns->req_map[idx].content));
		if (strlen(name) == strlen(ns->req_map[idx].content) &&
			strcmp(name, ns->req_map[idx].content, strlen(name)) == 0) {
			debug(DEBUG_SYSCALL, "!!!!FOUND %s, current idx = %d, service = %s", name, idx, ns->req_map[idx].content);
			is_found = 1;
			*postion = idx;
			break;
		}
	}
	return (is_found ? SERVER_ERR_SUCCESS : SERVER_ERR_NOT_REGISTERED);
}

/*
 * Insert a request into the req_map.
 * Return SERVER_ERR_MAX_NUM_NAMES_REACHED if there are two names associated with the task.
 * Return SERVER_ERR_ALREADY_REGISTERED if the task is already registered.
 * Overwrite the <name, tid> pair if neccessary.
 */
static Server_err insert_service(Name_server *ns, Name_server_message *req)
{
	debug(DEBUG_SYSCALL, "enter %s", "insert_service");
	int tid = req->tid;
	if (ns->tid_filled[tid] == 2) {
		// task already has two names associate with it
		return SERVER_ERR_MAX_NUM_NAMES_REACHED;
	}

	// Check whether there is another task reqistered under the same name
	int postion;
	int result = locate_service(ns, req->content, &postion);
	debug(DEBUG_SYSCALL, "result = %d", result);
	if (result == SERVER_ERR_SUCCESS) {
		// another task is reqistered under the same name, overwrites it
		Name_server_message *old_req = &(ns->req_map[postion]);
		if (old_req->tid == req->tid) {
			// already registered task
			return SERVER_ERR_ALREADY_REGISTERED;
		}
		old_req->tid = req->tid;
		old_req->type = req->type;
	}
	else {
		// no task is reqistered under the same name
		Name_server_message *entry = &(ns->req_map[ns->req_map_pos]);
		entry->tid = req->tid;
		entry->type = req->type;
		memcpy(entry->content, req->content, strlen(req->content) + 1);
		ns->req_map_pos++;
		ns->tid_filled[tid]++;
		debug(DEBUG_SYSCALL, "entry->content = %s, ns->req_map_pos = %d, ns->tid_filled = %d",
								entry->content, ns->req_map_pos, ns->tid_filled[tid]);
	}
	return SERVER_ERR_SUCCESS;
}

void name_server_start()
{
	debug(DEBUG_SYSCALL, "Enter %s", "name_server_start");

	Name_server ns;
	initialize(&ns);

	while(1) {
		int tid;
		Name_server_message request;
		Receive(&tid, &request, sizeof(request));
		
		debug(DEBUG_SYSCALL, "request = %s", request.content);

		Server_err result = SERVER_ERR_FAILURE;
		int position = 0;
		Name_server_message reply;
		switch(request.type) {
			case MSG_REGITSER_AS:
				debug(DEBUG_SYSCALL, "%s", "MSG_REGITSER_AS");
				result = insert_service(&ns, &request);
				reply.tid = request.tid;
				if (result != SERVER_ERR_SUCCESS) {
					reply.type = MSG_FAILURE;
					reply.content[0] = result;
					reply.content[1] = '\0'; 
				} else {
					reply.type = MSG_SUCCESS;
					reply.content[0] = '\0';
				}
				debug(DEBUG_SYSCALL, "reply.tid = %d, reply.type = %d, reply.content = %d", reply.tid, reply.type, reply.content[0]);
				Reply(request.tid, &reply, sizeof(reply));
		 		break;
			case MSG_WHO_IS:
				debug(DEBUG_SYSCALL, "%s", "MSG_WHO_IS");
				result = locate_service(&ns, request.content, &position);
				reply.tid = request.tid;
				if (result != SERVER_ERR_SUCCESS) {
					reply.type = MSG_FAILURE;
					reply.content[0] = result;
					reply.content[1] = '\0'; 
				} else {
					reply.type = MSG_SUCCESS;
					reply.content[0] = ns.req_map[position].tid;
					reply.content[1] = '\0';
				}
				debug(DEBUG_SYSCALL, "reply.tid = %d, reply.type = %d, reply.content = %d", reply.tid, reply.type, reply.content[0]);
				Reply(request.tid, &reply, sizeof(reply));
		 		break;
			default:
				break;
		}
	}
}

int RegisterAs(char *name)
{
	debug(DEBUG_SYSCALL, "Enter %s, %s, %d", "RegisterAs", name, strlen(name) + 1);

	int result = 0;

	int name_server_tid;
	READ_NAME_SERVER_TID(&name_server_tid);
	debug(DEBUG_SYSCALL, "name_server_tid = %d", name_server_tid);

	Name_server_message req;
	req.tid = MyTid();
	req.type = MSG_REGITSER_AS;
	memcpy(req.content, name, strlen(name) + 1);
	debug(DEBUG_SYSCALL, "req.content = %s", req.content);
	Name_server_message reply;
 
	result = Send(name_server_tid, &req, sizeof(req), &reply, sizeof(reply));
	if (!result) {
		// Send successful	
		if (reply.type == MSG_SUCCESS) {
			debug(DEBUG_SYSCALL, "%s", "MSG_SUCCESS");
			return 0;
		}
		else {
			// MSG_REGITSER_AS_FAILURE
			debug(DEBUG_SYSCALL, "%s, return %d", "MSG_FAILURE", reply.content[0]);
			return reply.content[0];
		}
	}
	return result;
}

int WhoIs(char *name)
{
	debug(DEBUG_SYSCALL, "Enter %s, name = %s, size = %d", "WhoIs", name, strlen(name) + 1);
	int result = 0;

	int name_server_tid;
	READ_NAME_SERVER_TID(&name_server_tid);
	debug(DEBUG_SYSCALL, "name_server_tid = %d", name_server_tid);

	Name_server_message req;
	req.tid = MyTid();
	req.type = MSG_WHO_IS;
	memcpy(req.content, name, strlen(name) + 1);
	debug(DEBUG_SYSCALL, "req.content = %s", req.content);
	Name_server_message reply;
 
	result = Send(name_server_tid, &req, sizeof(req), &reply, sizeof(reply));
	if (!result) {
		// Send successful	
		if (reply.type == MSG_SUCCESS) {
			debug(DEBUG_SYSCALL, "%s, return = %d", "MSG_SUCCESS", reply.content);
			return reply.content[0]; // return tid associated with the name
		}
		else {
			// MSG_FAILURE
			return reply.content[0];
			debug(DEBUG_SYSCALL, "%s, return %d", "MSG_FAILURE", reply.content[0]);
		}
	}
	return result;
}
