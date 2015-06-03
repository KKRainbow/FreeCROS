#pragma once
#include"Global.h"
#include"SystemCallEntry.h"

#define SYSCALL_METHOD_H(callname,callnum) \
class SysCall##callname:public SysCall { \
	public:\
		static const int Num = callnum; \
		static int GetCallNum(){return Num;}\
		virtual int SystemCallNum()override; \
		virtual int Call(uint32_t _First,uint32_t Sec,\
				uint32_t _Third,uint32_t _Fourth,InterruptParams& params)override; \
		static inline int Invoke(uint32_t _First,uint32_t _Sec,\
				uint32_t _Third,uint32_t _Fourth) \
		{\
			int res = 0; \
			__asm__ volatile("int %1":"=a"(res):"i"(SYSCALL_IRQ_NUM),"a"(GetCallNum()),"b"(_First)\
					,"c"(_Sec),"d"(_Third),"S"(_Fourth));\
			return res;\
		}\
}

#define SYSCALL_METHOD_CPP(callname) \
	int SysCall##callname::SystemCallNum(){return SysCall##callname::GetCallNum();}\
	int SysCall##callname::Call(uint32_t _First ,uint32_t _Sec,\
				uint32_t _Third,uint32_t _Fourth,InterruptParams& params)

SYSCALL_METHOD_H(Log,0);
SYSCALL_METHOD_H(CreateThread,1);  //Entry
SYSCALL_METHOD_H(WriteToPhisicalAddr,2);//Dest Src Size
SYSCALL_METHOD_H(ReadFromPhisicalAddr,3);//Dest Src Size
SYSCALL_METHOD_H(SendMessageTo,4); //Message*
SYSCALL_METHOD_H(ReceiveFrom,5); //Source,Message*
SYSCALL_METHOD_H(ReceiveAll,6); //Message*
SYSCALL_METHOD_H(ReadDataFromThread,7); //PID,Addr,Size
SYSCALL_METHOD_H(RegisterIRQ,8); //irqnum

