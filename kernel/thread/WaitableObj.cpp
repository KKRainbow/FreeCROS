#include"WaitableObj.h"
#include"CPUManger.h"
#include"ThreadManager.h"


void WaitableObj::Wait()
{
	lock.Lock();
	CPU* cpu = CPUManager::Instance()->GetCurrentCPU();
	Thread* prev = wait;
	Thread* curr = cpu->GetCurrThreadRunning();

	wait = curr;
	
	curr->State()->ToPause(curr);
	cpu->ExhaustCurrThread(); //当前进程的时间片归零	
	cpu->Run();

	wait = prev;
	if(prev != nullptr)
	{
		prev->State()->ToReady(prev);
	}
	lock.Unlock();
}
void WaitableObj::Wake()
{
	lock.Lock();
	if(wait)wait->State()->ToReady(wait);
	lock.Unlock();
}
void WaitableObj::Sleep()
{
	lock.Lock();
	CPU* cpu = CPUManager::Instance()->GetCurrentCPU();
	Thread* prev = wait;
	Thread* curr = cpu->GetCurrThreadRunning();

	wait = curr;
	
	curr->State()->ToIOBlocked(curr);
	cpu->ExhaustCurrThread(); //当前进程的时间片归零	
	cpu->Run();

	wait = prev;
	if(prev != nullptr)
	{
		prev->State()->ToReady(prev);
	}
	lock.Unlock();
}
