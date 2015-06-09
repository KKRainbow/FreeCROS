#include"ThreadState.h"
#include"ThreadManager.h"
#include"Scheduler.h"
ThreadState* ThreadState::GetState(States _State)
{
	switch(_State)
	{
		case RUNNING:
			return new ThreadStateRunning();
		case READY:
			return new ThreadStateReady();
		case INTERRUPTABLE:
			return new ThreadStateInterruptable();
		case UNINTERRUPTABLE:
			return new ThreadStateUninterruptable();
		case ZOMBIE:
			return new ThreadStateZombie();
		default:
			return nullptr;
	}
}
bool ThreadStateUninterruptable::ToRun(Thread* _Thread)
{
	return false;
}
bool ThreadStateUninterruptable::ToIOBlocked(Thread* _Thread)
{
	return false;
}
bool ThreadStateUninterruptable::ToPause(Thread* _Thread)
{
	return false;
}
bool ThreadStateUninterruptable::ToReady(Thread* _Thread)
{
	_Thread->State() = new ThreadStateReady();
	ThreadManager::Instance()->sched->ThreadAdded(_Thread);
	return true;
}
bool ThreadStateUninterruptable::ToZombie(Thread* _Thread)
{
	return false;
}
States ThreadStateUninterruptable::Type()
{
	return States::UNINTERRUPTABLE;
}
bool ThreadStateInterruptable::ToRun(Thread* _Thread)
{
	_Thread->PrepareToRun();			//Make the thread prepare something
	return false;
}
bool ThreadStateInterruptable::ToIOBlocked(Thread* _Thread)
{
	return false;
}
bool ThreadStateInterruptable::ToPause(Thread* _Thread)
{
	return false;
}
bool ThreadStateInterruptable::ToReady(Thread* _Thread)
{
	_Thread->State() = new ThreadStateReady();
	ThreadManager::Instance()->sched->ThreadAdded(_Thread);
	return true;
}
bool ThreadStateInterruptable::ToZombie(Thread* _Thread)
{
	return false;
}
States ThreadStateInterruptable::Type()
{
	return States::INTERRUPTABLE;
}
bool ThreadStateReady::ToRun(Thread* _Thread)
{
	_Thread->State() = new ThreadStateRunning();
	ThreadManager::Instance()->sched->ThreadRemoved(_Thread);
	_Thread->PrepareToRun();			//Make the thread prepare something
	return true;
}
bool ThreadStateReady::ToIOBlocked(Thread* _Thread)
{
	return false;
}
bool ThreadStateReady::ToPause(Thread* _Thread)
{
	_Thread->State() = new ThreadStateInterruptable();
	ThreadManager::Instance()->sched->ThreadRemoved(_Thread);
	return true;
}
bool ThreadStateReady::ToReady(Thread* _Thread)
{
	return false;
}
bool ThreadStateReady::ToZombie(Thread* _Thread)
{
	return false;
}
States ThreadStateReady::Type()
{
	return States::READY;
}
bool ThreadStateZombie::ToRun(Thread* _Thread)
{
	return false;
}
bool ThreadStateZombie::ToIOBlocked(Thread* _Thread)
{
	return false;
}
bool ThreadStateZombie::ToPause(Thread* _Thread)
{
	return false;
}
bool ThreadStateZombie::ToReady(Thread* _Thread)
{
	return false;
}
bool ThreadStateZombie::ToZombie(Thread* _Thread)
{
	return false;
}
States ThreadStateZombie::Type()
{
	return States::ZOMBIE;
}
bool ThreadStateRunning::ToRun(Thread* _Thread)
{
	return false;
}
bool ThreadStateRunning::ToIOBlocked(Thread* _Thread)
{
	_Thread->State() = new ThreadStateUninterruptable();
	return true;
}
bool ThreadStateRunning::ToPause(Thread* _Thread)
{
	_Thread->State() = new ThreadStateInterruptable();
	return true;
}
bool ThreadStateRunning::ToReady(Thread* _Thread)
{
	_Thread->State() = new ThreadStateReady();
	ThreadManager::Instance()->sched->ThreadAdded(_Thread);
	return true;
}
bool ThreadStateRunning::ToZombie(Thread* _Thread)
{
	return false;
}
States ThreadStateRunning::Type()
{
	return States::RUNNING;
}
