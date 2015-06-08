#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
#include"Device.h"
void thread();
int main()
{
	Message msg;
	int i = 0;
	char a[500];
	
	int pid = SysCallCreateThread::Invoke((uint32_t)thread,0,0,0);
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

void thread()
{
	char a[500];
	Message msg;
	Message ret;
	int i = 0;
	
	auto id = SysCallRegisterChrDev::Invoke((uint32_t)"keyboard",0,0,0);
	log("my device id: %d\n",id);
// 	for(int i = 0;i<0x1234560;i++);//等待
	for(;;)
	{
		SysCallReceiveAll::Invoke((uint32_t)&msg,0,0,0);
		switch(msg.content[0])
		{
			case MSG_DEVICE_OPEN_OPERATION :
				//log("Get Open Operation From : %d\n",msg.source);
				ret.destination = msg.source;
				ret.content[0] = 1;
				SysCallSendMessageTo::Invoke((uint32_t)&ret);
				break;
			case MSG_DEVICE_READ_OPERATION :
				log("Get Read Operation From : %d\n",msg.source);
				char r[] = "Hello I'm KeyBoard\n";
				ret.destination = msg.source;
				ret.content[0] = (uint32_t)r;
				ret.content[1] = sizeof(r);
				SysCallSendMessageTo::Invoke((uint32_t)&ret);
				break;
		}
		i++;
	}
}
