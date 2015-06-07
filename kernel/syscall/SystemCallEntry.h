#pragma once
#include"Global.h"
#include"Interrupt.h"
#include"stl/smap.h"
#include"cpu/SpinLock.h"

class SystemCallEntry
{
	SINGLETON_H(SystemCallEntry)
	private:
		lr::sstl::Map<int,SysCall*> callMap;
	public:
		static int Call(InterruptParams& params);
};
