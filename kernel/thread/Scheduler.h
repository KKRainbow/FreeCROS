#pragma once
#include"Global.h"
class Thread;
class Scheduler
{
	public:
		const int MIN_PRIORITY = 0;
		const int MAX_PRIORITY = 100;
		virtual void Init() = 0;
		virtual void Deinit() = 0;
		virtual void ThreadAdded(Thread* thread) = 0;
		virtual void ThreadRemoved(Thread* thread) = 0; 
		virtual Thread* NextThread() = 0;
		virtual void RaisePriority(Thread* thread,int _Pri) = 0;
		virtual void DownPriority(Thread* thread,int _Pri) = 0;
};
