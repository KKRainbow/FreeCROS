#include"ThreadManager.h"
#include"SchedulerDefault.h"
#include"Log.h"
using namespace lr::sstl;

SINGLETON_CPP(ThreadManager)
{
	lock.Lock();

	this->sched = new SchedulerDefault();
	this->sched->Init();

	lock.Unlock();
}
Thread* ThreadManager::CreateThread(ThreadType _Type)
{
	lock.Lock();

	Thread* thread = new Thread(pidGen.GetID(),_Type);
	this->threads.Insert(MakePair(thread->GetPid(),thread));

	lock.Unlock();
	return thread;
}
Thread* ThreadManager::CreateChildThread(Thread* _Parent)
{
	lock.Lock();

	Thread* thread = new Thread(*_Parent,pidGen.GetID());
	this->threads.Insert(MakePair(thread->GetPid(),thread));

	lock.Unlock();
	return thread;

}
void ThreadManager::RemoveThread(int _Pid)
{
	lock.Lock();

	auto ite = threads.Find(_Pid);
	if(ite == threads.End())
	{
		lock.Unlock();
		return;
	}

	Thread* thread = ite->second;
	threads.Erase(ite);
	sched->ThreadRemoved(thread);

	delete thread;

	lock.Unlock();
}
Thread* ThreadManager::GetNextThreadToExecute(CPU* _CPU)
{
	lock.Lock();

	Thread* res  =sched->NextThread(_CPU);
	if(res)
		res->State()->ToRun(res);

	lock.Unlock();
	return res;
}
Thread* ThreadManager::GetThreadByPID(int _Pid)
{
	lock.Lock();

	auto ite = threads.Find(_Pid);
	if(ite == threads.End())
	{
		lock.Unlock();
		return nullptr;
	}

	Thread* thread = ite->second;

	lock.Unlock();
	return thread;	
}
void ThreadManager::SendMessage(IPCMessage& _Msg)
{
	lock.Lock();

	GetThreadByPID(_Msg.GetDestination())->ReceiveMessage(_Msg);

	lock.Unlock();
}

void ThreadManager::SetPriority(Thread* _Thread,int _Priority)
{
	auto old = _Thread->Priority();
	if(_Priority == old)return;
	
	lock.Lock();
	if(_Priority > old)
	{
		this->sched->RaisePriority(_Thread,_Priority);
	}
	else
	{
		this->sched->DownPriority(_Thread,_Priority);
	}
	lock.Unlock();
}