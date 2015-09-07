#pragma once

#include"UserLog.h"
#include"SystemCalls.h"
#include"Threads.h"
#define MSG_DEVICE_OPEN_OPERATION 10
#define MSG_DEVICE_WRITE_OPERATION 11
#define MSG_DEVICE_READ_OPERATION 12
#define MSG_DEVICE_SEEK_OPERATION 13
#define MSG_DEVICE_MKDIR_OPERATION 14

#define DEFINE_FUNC(Oper) \
static void Dev##Oper() \
{ \
Message ret; \
Message msg = globalMsg; \
ret = globalDevOp-> \
Oper (msg); \
ret.destination = msg.source; \
SysCallSendMessageTo::Invoke((uint32_t)&ret, SEND_MESSAGE_FLAG_PROXY_FATHER); \
SysCallExit::Invoke(); \
}

class DeviceOperation
{
public:
	virtual Message Open(Message& _Msg) = 0;
	virtual Message Read(Message& _Msg) = 0;
	virtual Message Write(Message& _Msg) = 0;
	virtual Message Other(Message& _Msg) = 0;
};
static DeviceOperation* globalDevOp = nullptr;
static Message globalMsg;

DEFINE_FUNC(Open)
DEFINE_FUNC(Read)
DEFINE_FUNC(Write)
DEFINE_FUNC(Other)

static void device_loop(DeviceOperation& _Op)
{
	char a[500];
	int i = 0;
	globalDevOp = &_Op;
	int pid;
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
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
			case MSG_DEVICE_WRITE_OPERATION :
				pid = SysCallCreateThread::Invoke((uint32_t) DevWrite);
				break;
			default:
				pid = SysCallCreateThread::Invoke((uint32_t) DevOther);
				break;
		}
		i++;
	}
#pragma clang diagnostic pop
}

