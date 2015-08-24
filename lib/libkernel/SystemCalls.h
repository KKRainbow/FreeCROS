#pragma once
#include"Global.h"
#include"Interrupts.h"

#define SYSCALL_IRQ_NUM 0x80

#define SYSCALL_METHOD_H(callname,callnum) \
class SysCall##callname:public SysCall { \
	public:\
		static const int Num = callnum; \
		static int GetCallNum(){return Num;}\
		virtual int SystemCallNum()override; \
		virtual int Call(uint32_t _First,uint32_t Sec,\
				uint32_t _Third,uint32_t _Fourth,InterruptParams& params)override; \
		static inline int Invoke(uint32_t _First = 0,uint32_t _Sec = 0,\
				uint32_t _Third = 0,uint32_t _Fourth = 0) \
		{\
			int res = 0; \
			__asm__ volatile("int %1":"=a"(res):"i"(SYSCALL_IRQ_NUM),"a"(GetCallNum()),"b"(_First)\
					,"c"(_Sec),"d"(_Third),"S"(_Fourth));\
			return res;\
		}\
};\
static inline int callname(uint32_t _First = 0,uint32_t _Sec = 0,\
				uint32_t _Third = 0,uint32_t _Fourth = 0) \
{\
	return SysCall##callname::Invoke(_First,_Sec,_Third,_Fourth);\
}

#define SYSCALL_METHOD_CPP(callname) \
	int SysCall##callname::SystemCallNum(){return SysCall##callname::GetCallNum();}\
	int SysCall##callname::Call(uint32_t _First ,uint32_t _Sec,\
				uint32_t _Third,uint32_t _Fourth,InterruptParams& params)

class SysCall
{
	public:
		virtual int SystemCallNum() = 0;
		virtual int Call(uint32_t _First,uint32_t _Sec,
				uint32_t _Third,uint32_t _Fourth,
				InterruptParams& params) = 0;
		//Four parameters
};

SYSCALL_METHOD_H(Log,0);
SYSCALL_METHOD_H(CreateThread,1);  //Entry
SYSCALL_METHOD_H(WriteToPhisicalAddr,2);//Dest Src Size
SYSCALL_METHOD_H(ReadFromPhisicalAddr,3);//Dest Src Size

#define SEND_MESSAGE_FLAG_PROXY_PROCESS 1
#define SEND_MESSAGE_FLAG_PROXY_FATHER 2
SYSCALL_METHOD_H(SendMessageTo,4); //Message*
SYSCALL_METHOD_H(ReceiveFrom,5); //Source,Message*
SYSCALL_METHOD_H(ReceiveAll,6); //Message*
SYSCALL_METHOD_H(ReadDataFromThread,7); //PID,Addr,Size
SYSCALL_METHOD_H(RegisterIRQ,8); //irqnum
SYSCALL_METHOD_H(RegisterChrDev,9); //devname
SYSCALL_METHOD_H(Open,10); //devname
SYSCALL_METHOD_H(Read,11); //devname
SYSCALL_METHOD_H(Write,12); //devname
SYSCALL_METHOD_H(Seek,13); //devname

SYSCALL_METHOD_H(Signal,18); //signum,handler,flag
SYSCALL_METHOD_H(Kill,19); //pid,signum
SYSCALL_METHOD_H(SignalRestore,20);//no params
SYSCALL_METHOD_H(Alarm,21); //_Us


SYSCALL_METHOD_H(Pause,22);
SYSCALL_METHOD_H(Sleep,23);
SYSCALL_METHOD_H(WakeUp,24);
SYSCALL_METHOD_H(Exit,25);
SYSCALL_METHOD_H(GiveUp,26);

SYSCALL_METHOD_H(MountFs,27);
SYSCALL_METHOD_H(Mkdir,28);
