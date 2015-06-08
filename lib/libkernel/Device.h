#pragma once

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


static void device_loop(DeviceOperation& _Op)
{
	char a[500];
	char tmpbuffer[500];
	Message msg;
	Message ret;
	int i = 0;
	for(;;)
	{
		SysCallReceiveAll::Invoke((uint32_t)&msg,0,0,0);
		switch(msg.content[0])
		{
			case MSG_DEVICE_OPEN_OPERATION :
				//log("Get Open Operation From : %d\n",msg.source);
				ret.destination = msg.source;
				ret.content[0] = _Op.Open(msg.source);
				SysCallSendMessageTo::Invoke((uint32_t)&ret);
				break;
			case MSG_DEVICE_READ_OPERATION :
				_Op.Read(tmpbuffer,msg.content[2]);
				
				ret.destination = msg.source;
				ret.content[0] = (uint32_t)tmpbuffer;
				ret.content[1] = sizeof(tmpbuffer);
				SysCallSendMessageTo::Invoke((uint32_t)&ret);
				break;
		}
		i++;
	}
}