#ifndef __USER_FUNCTIONS_H__
#define __USER_FUNCTIONS_H__
//void init_kernel();
int Create(int priority, void (*code) ());
void Pass();
void Exit();
int MyTid();
int MyParentTid();
int Send( int tid, void *msg, int msglen, void *reply, int replylen );
int Receive( int *tid, void *msg, int msglen );
int Reply( int tid, void *reply, int replylen );
int AwaitEvent(int eventType);
#endif // __USER_FUNCTIONS_H__
