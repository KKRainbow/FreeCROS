#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
#include"Device.h"

class DeviceOperationKeyboard : public DeviceOperation
{
public:
	virtual	bool Open(pid_t _Pid)
	{
		return true;
	}
	virtual	size_t Read(char* _Buffer,size_t _Size)
	{
		char a[] = "Keyboard test";
		memcpy(_Buffer,a,sizeof(a));
		return sizeof(a);
	}
	virtual	size_t Write(char* _Buffer,size_t _Size)
	{
		
	}
};


void keyboard()
{
	auto id = SysCallRegisterChrDev::Invoke((uint32_t)"keyboard",0,0,0);
	DeviceOperationKeyboard kop;
	device_loop(kop);
}