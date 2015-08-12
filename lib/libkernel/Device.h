#pragma once

#include"UserLog.h"
#include"SystemCalls.h"
#include"Threads.h"
#define MSG_DEVICE_OPEN_OPERATION 10
#define MSG_DEVICE_WRITE_OPERATION 11
#define MSG_DEVICE_READ_OPERATION 12
#define MSG_DEVICE_SEEK_OPERATION 13

class DeviceOperation
{
public:
	virtual bool Open(pid_t _Pid) = 0;
	virtual size_t Read(char* _Buffer,size_t _Size) = 0;
	virtual size_t Write(char* _Buffer,size_t _Size) = 0;
};
static DeviceOperation* globalDevOp = nullptr;
static Message globalMsg;
static void DevOpen()
{
	Message ret;
	Message msg = globalMsg;
	ret.destination = msg.source;
	ret.content[0] = (uint32_t)globalDevOp->Open(msg.source);
	SysCallSendMessageTo::Invoke((uint32_t)&ret, SEND_MESSAGE_FLAG_PROXY_FATHER);
	log("open~~~~!!!!~~~\n");
	SysCallExit::Invoke();
}

static void DevRead()
{
	Message ret;
	Message msg = globalMsg;
	char* tmpbuffer = new char[msg.content[2]];
	tmpbuffer[3] = 0;

	ret.destination = msg.source;
	ret.content[0] = (uint32_t)tmpbuffer;
	ret.content[1] = (uint32_t)globalDevOp->Read(tmpbuffer,msg.content[2]);
	delete tmpbuffer;
	SysCallSendMessageTo::Invoke((uint32_t)&ret, SEND_MESSAGE_FLAG_PROXY_FATHER);
	SysCallExit::Invoke();
}

static void device_loop(DeviceOperation& _Op)
{
	char a[500];
	char tmpbuffer[500];
	Message msg;
	int i = 0;
	globalDevOp = &_Op;
	int pid;
	for(;;)
	{
		SysCallReceiveAll::Invoke((uint32_t)&globalMsg,0,0,0);
		switch(globalMsg.content[0])
		{
			case MSG_DEVICE_OPEN_OPERATION :
				//log("Get Open Operation From : %d\n",msg.source);
				pid = SysCallCreateThread::Invoke((uint32_t) DevOpen);
				break;
			case MSG_DEVICE_READ_OPERATION :
				pid = SysCallCreateThread::Invoke((uint32_t) DevRead);
				break;
		}
		i++;
	}
}

