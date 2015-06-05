#pragma once
#include"Global.h"
#include"Thread.h"
#include"stl/sidgen.h"
#include"cpu/CPU.h"

class Scheduler;
class ThreadManager
{
	SINGLETON_H(ThreadManager)
	private:
		friend class ThreadStateReady;
		friend class ThreadStateZombie;
		friend class ThreadStateRunning;
		friend class ThreadStateInterruptable;
		friend class ThreadStateUninterruptable;
	private:
		lr::sstl::IDGenerator<int> pidGen;
		lr::sstl::Map<int,Thread*> threads;
		Scheduler* sched;
		SpinLock lock;
	public:
		Thread* CreateThread(ThreadType _Type);
		Thread* CreateChildThread(Thread* _Parent);
		void RemoveThread(int _Pid);
		Thread* GetNextThreadToExecute(CPU* _CPU);
		Thread* GetThreadByPID(int _Pid);
		void SendMessage(IPCMessage& _Msg);
		void SetPriority(Thread* _Thread,int _Priority);
		void ClockNotify(uint64_t _Counter);
};

