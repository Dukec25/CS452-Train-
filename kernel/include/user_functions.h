#ifndef __USER_FUNCTIONS_H__
#define __USER_FUNCTIONS_H__
int Create(int priority, void (*code) ());
void Pass();
void Exit();
int MyTid();
int MyParentTid();

int Send( int tid, void *msg, int msglen, void *reply, int replylen );
int Receive( int *tid, void *msg, int msglen );
int Reply( int tid, void *reply, int replylen );

int DelayUntil( int ticks );
int Delay( int ticks );
int Time();

int AwaitEvent(int eventType, char ch);

int Getc(int channel);
int Putc(int channel, char ch);
#endif // __USER_FUNCTIONS_H__
