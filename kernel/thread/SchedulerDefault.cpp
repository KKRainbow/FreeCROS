#include"SchedulerDefault.h"
void SchedulerDefault::Init()
{

}

void SchedulerDefault::Deinit()
{

}

void SchedulerDefault::ThreadAdded(Thread* thread)
{
	lock.Lock();
	int priority = thread->Priority()/(100/MAX_LIST_SIZE);
	if(priority>=MAX_LIST_SIZE)priority = MAX_LIST_SIZE-1;
	if(priority<0)priority = 0;
	this->lists[priority].PushBack(thread);
	lock.Unlock();
}

void SchedulerDefault::ThreadRemoved(Thread* thread)
{
	lock.Lock();
	decltype(lists[0].Begin()) ite;
	bool flag = false;
	int i = 0;
	for(i = 0;i<MAX_LIST_SIZE;i++)
	{
		ite = lr::sstl::find(lists[i].Begin(),lists[i].End(),thread);
		if(ite != lists[i].End())
		{
			flag = true;
			break;
		}
	}
	if(flag)
	{
		lists[i].Erase(ite);
	}
	lock.Unlock();
}

Thread* SchedulerDefault::NextThread(CPU* _CPU)
{
	lock.Lock();
	decltype(lists[0].Begin()) ite;
	int i = 0;
	for(i = MAX_LIST_SIZE-1;i>= 0 ;i--)
	{
		ite = lists[i].Begin();
		if(ite != lists[i].End()) //List of this priority is not empty.
		{
			Thread* res = *ite;
			
			if(res->threadLock.Try() == false)break;
			
			Assert(res);
			auto& state = res->State();
			Assert(state.Obj());
			auto stateType = state->Type();
			Assert(stateType == States::READY || stateType == States::INTERRUPTABLE);
			
			//在这里就得把这个线程从准备列表中山除了,不然另一个CPU过来还有可能选择它;
			lock.Unlock();
			this->ThreadRemoved(res);
			return res;
		}
	}
	lock.Unlock();
	return nullptr;
}

void SchedulerDefault::RaisePriority(Thread* thread,int _Pri)
{
	lock.Lock();
	ThreadRemoved(thread);
	thread->Priority() = _Pri;
	ThreadAdded(thread);
	lock.Unlock();
}

void SchedulerDefault::DownPriority(Thread* thread,int _Pri)
{
	lock.Lock();
	ThreadRemoved(thread);
	thread->Priority() = _Pri;
	ThreadAdded(thread);
	lock.Unlock();
}



