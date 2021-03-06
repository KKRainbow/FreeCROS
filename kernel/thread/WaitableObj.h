#pragma once
#include"Global.h"
#include"cpu/SpinLock.h"

class Thread;
class WaitableObj
{
	private:
		Thread* wait = nullptr;
		SpinLock lock;
	public:
		void Wait();
		void Sleep();
		void Wake(); 
};
