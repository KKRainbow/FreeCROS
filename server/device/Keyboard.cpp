#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
#include"Device.h"

class DeviceOperationKeyboard : public DeviceOperation
{
public:
	DeviceOperationKeyboard()
	{
		
	}
	virtual	bool Open(pid_t _Pid)
	{
		return true;
	}
	virtual	size_t Read(char* _Buffer,size_t _Size)
	{
		char a[] = "Keyboard test\n";
		memcpy(_Buffer,a,sizeof(a));
		return sizeof(a);
	}
	virtual	size_t Write(char* _Buffer,size_t _Size)
	{
		
	}
};

void tmp(int num,void*,void*)
{
	log("signal\n");
}

void keyboard()
{
	Signal(SIGALARM,(sighandler_t)tmp,0);
	Signal(SIGINT ,(sighandler_t)tmp,0);
	SysCallAlarm::Invoke(1e3);
	SysCallRegisterIRQ::Invoke(65);
	auto id = SysCallRegisterChrDev::Invoke((uint32_t)"keyboard",0,0,0);
	log("open id: %d\n",id);
	DeviceOperationKeyboard kop;
	device_loop(kop);
}