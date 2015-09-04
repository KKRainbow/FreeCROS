#include"WaitableObj.h"
#include"cpu/CPUManager.h"
#include"ThreadManager.h"


void WaitableObj::Wait() {
    uint32_t eflags;
    Interrupt::EnterCritical(eflags);
    lock.Lock();
    CPU *cpu = CPUManager::Instance()->GetCurrentCPU();
    Thread *prev = wait;
    Thread *curr = cpu->GetCurrThreadRunning();

    wait = curr;

    curr->State()->ToPause(curr);
    lock.Unlock();
    repeat:
    cpu->ExhaustCurrThread(); //当前进程的时间片归零
    cpu->Run();
    if ( curr->State()->Type() == States::UNINTERRUPTABLE ) {
        goto repeat;
    }
    lock.Lock();

    wait = prev;
    if ( prev != nullptr ) {
        prev->State()->ToReady(prev);
    }
    lock.Unlock();
    Interrupt::LeaveCritical(eflags);
}

void WaitableObj::Wake() {
    lock.Lock();
    if ( wait )wait->State()->ToReady(wait);
    lock.Unlock();
}

void WaitableObj::Sleep() {
    lock.Lock();
    CPU *cpu = CPUManager::Instance()->GetCurrentCPU();
    Thread *prev = wait;
    Thread *curr = cpu->GetCurrThreadRunning();
    wait = curr;
    curr->State()->ToIOBlocked(curr);
    lock.Unlock();
    repeat:
    cpu->ExhaustCurrThread(); //当前进程的时间片归零
    cpu->Run();
    if ( curr->State()->Type() == States::UNINTERRUPTABLE ) {
        goto repeat;
    }
    lock.Lock();

    wait = prev;
    if ( prev != nullptr ) {
        prev->State()->ToReady(prev);
    }
    lock.Unlock();
}
