#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
void thread();
int main()
{
	Message msg;
	int i = 0;
	char a[500];
	int pid = SysCallCreateThread::Invoke((uint32_t)thread,0,0,0);
	log("pid: %d\n",pid);
	for(;;)
	{
		msg.content[0] = i;
		msg.destination = pid;
		SysCallSendMessageTo::Invoke((uint32_t)&msg,0,0,0);
		i++;
	}
	return 1;
}

void thread()
{
	__asm__("xchg %%bx,%%bx\n\t"::);
	char a[500];
	for(;;)
	{
		Message msg;
		SysCallReceiveAll::Invoke((uint32_t)&msg,0,0,0);
		int n  =snprintf(a,sizeof(a),"Message received, num: %d time: %d\n",msg.content[0]
				,(uint32_t)msg.timeStamp);
		SysCallLog::Invoke((uint32_t)a,n,0,0);	
	}
}
