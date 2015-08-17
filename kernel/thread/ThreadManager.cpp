#include"ThreadManager.h"
#include"SchedulerDefault.h"

using namespace lr::sstl;

SINGLETON_CPP(ThreadManager) {
    lock.Lock();

    this->sched = new SchedulerDefault();
    this->sched->Init();

    lock.Unlock();
}

Thread *ThreadManager::CreateThread(ThreadType _Type) {
    lock.Lock();

    Thread *thread = new Thread(pidGen.GetID(), _Type);
    this->threads.Insert(MakePair(thread->GetPid(), thread));

    lock.Unlock();
    return thread;
}

Thread *ThreadManager::CreateChildThread(Thread *_Parent) {
    lock.Lock();

    Thread *thread = new Thread(*_Parent, pidGen.GetID());
    this->threads.Insert(MakePair(thread->GetPid(), thread));

    //确保线程产生的线程也属于父线程所属的进程
    auto belongTo = _Parent->belongTo;
    thread->belongTo = belongTo ? belongTo : _Parent;

    lock.Unlock();
    return thread;

}

void ThreadManager::RemoveThread(int _Pid) {
    lock.Lock();

    auto ite = threads.Find(_Pid);
    if ( ite == threads.End()) {
        lock.Unlock();
        return;
    }

    Thread *thread = ite->second;
    sched->ThreadRemoved(thread);

    thread->State()->ToZombie(thread);
    this->zombieThreads.PushBack(thread);

    lock.Unlock();
}

Thread *ThreadManager::GetNextThreadToExecute(CPU *_CPU) {
    lock.Lock();
    //扫描线程列表,检查信号
    for ( auto &pair : this->threads ) {
        Thread *t = pair.second;
        if ( t->threadLock.Try()) {
            if ( t->sigmap.Size() != 0 ) {
                if ( t->State()->Type() == States::INTERRUPTABLE ) {
                    t->State()->ToReady(t);
                }
            }

            t->threadLock.Unlock();
        }
        //mask交给Thread类自己去判断
    }
    //删除所有僵尸进程
    for ( auto ite = this->zombieThreads.Begin(); ite != this->zombieThreads.End(); ite++ ) {
        Thread *zombie = *ite;
        if ( !zombie ) {
            this->zombieThreads.Erase(ite);
        }
        else if ( zombie->threadLock.Try()) {
            //开始删除
            auto parent = zombie->father;
            if ( zombie->mapSlot != -1 ) {
                zombie->childBitMap[zombie->mapSlot] = 0;
            }
            this->threads.Erase(this->threads.Find(zombie->GetPid()));
            this->zombieThreads.Erase(ite);
            ite = this->zombieThreads.Begin();
        }
        else {
            continue;
        }
    }
    Thread *res = sched->NextThread(_CPU);
    lock.Unlock();
    return res;
}

Thread *ThreadManager::GetThreadByPID(int _Pid) {
    lock.Lock();

    auto ite = threads.Find(_Pid);
    if ( ite == threads.End()) {
        lock.Unlock();
        return nullptr;
    }

    Thread *thread = ite->second;

    lock.Unlock();
    return thread;
}

void ThreadManager::SendMessage(IPCMessage &_Msg) {
    lock.Lock();

    GetThreadByPID(_Msg.GetDestination())->ReceiveMessage(_Msg);

    lock.Unlock();
}

void ThreadManager::SetPriority(Thread *_Thread, int _Priority) {
    auto old = _Thread->Priority();
    if ( _Priority == old )return;

    lock.Lock();
    if ( _Priority > old ) {
        this->sched->RaisePriority(_Thread, _Priority);
    }
    else {
        this->sched->DownPriority(_Thread, _Priority);
    }
    lock.Unlock();
}

void ThreadManager::ClockNotify(uint64_t _Counter) {
    for ( auto &pair : this->threads ) {
        Thread *thread = pair.second;
        if ( thread->threadLock.Try()) {
            thread->ClockNotify(_Counter);
            thread->threadLock.Unlock();
        }
    }
}

bool ThreadManager::IsOrphaned(pid_t _Pgrp) {
    return false;
}

void ThreadManager::KillProcessGroup(pid_t _Pgrp, int _Sig, int _Priv) {
    return;
}
