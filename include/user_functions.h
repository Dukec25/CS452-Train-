#ifndef __USER_FUNCTIONS_H__
#define __USER_FUNCTIONS_H__
void init_kernel();
int Create(int priority, void (*code) ());
void Pass();
void Exit();
int MyTid();
int MyParentTid();
#endif // __USER_FUNCTIONS_H__
