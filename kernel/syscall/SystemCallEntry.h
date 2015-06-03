#pragma once
#include"Global.h"
#include"Interrupt.h"
#include"stl/smap.h"
#include"cpu/SpinLock.h"
#define SYSCALL_IRQ_NUM 0x80
class SysCall
{
	protected:
		SpinLock lock;
	public:
		virtual int SystemCallNum() = 0;
		virtual int Call(uint32_t _First,uint32_t _Sec,
				uint32_t _Third,uint32_t _Fourth,
				InterruptParams& params) = 0;
		//Four parameters
};


class SystemCallEntry
{
	SINGLETON_H(SystemCallEntry)
	private:
		lr::sstl::Map<int,SysCall*> callMap;
	public:
		static int Call(InterruptParams& params);
};
