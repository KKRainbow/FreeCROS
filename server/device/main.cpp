#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
#include"Device.h"
void keyboard();
int main()
{
	Message msg;
	int i = 0;
	char a[500];
	
	int pid = SysCallCreateThread::Invoke((uint32_t)keyboard,0,0,0);
	log("pid: %d\n",pid);
	
	for(int i = 0;i<0x123456;i++);//等待
// 	SysCallSendMessageTo::Invoke((uint32_t)&msg,0,0,0);
	log("start open\n");
	auto fid = SysCallOpen::Invoke((uint32_t)"/dev/keyboard",0,0,0);
	log("Get dev_t : %d\n",fid);
	
	auto size = SysCallRead::Invoke(fid,(uint32_t)a,500,0);
	log("\nReceived from deviceabdd\n");
	log(a);
	for(;;);
	return 1;
}
