#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
#include"Device.h"
#include"KeyHandler.h"


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
		char a[2] = " ";
		a[0] = read_key().ascii;
		memcpy(_Buffer,a,sizeof(a));
		return sizeof(a);
	}
	virtual	size_t Write(char* _Buffer,size_t _Size)
	{
		
	}
};

void KeyboardIntHandler(int num,void*,void*)
{
	auto scanCode = Inb(0x60);
	do_intr(scanCode);
	//清空buffer
	auto tmp = Inb(0x61);
	Outb(tmp | 0x80, 0x61);
	Outb(tmp & 0x7f, 0x61);
	Outb(0x20,0x61);
}
void keyboard()
{
	Signal(SIGINT ,(sighandler_t)KeyboardIntHandler,0);
// 	SysCallAlarm::Invoke(1e3);
	SysCallRegisterIRQ::Invoke(65);
	log("trying open\n");
	auto id = SysCallRegisterChrDev::Invoke((uint32_t)"keyboard",0,0,0);
	log("open id: %d\n",id);
	DeviceOperationKeyboard kop;
	device_loop(kop);
}