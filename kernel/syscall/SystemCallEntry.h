#pragma once
#include"Global.h"
#include"Interrupt.h"
#include"stl/smap.h"
#include"SpinLock.h"
#include"SystemCalls.h"

class SystemCallEntry
{
	SINGLETON_H(SystemCallEntry)
	SpinLock lock;
	private:
		lr::sstl::Map<int,SysCall*> callMap;
	public:
		static int Call(InterruptParams& params);
};
