#pragma once

#include"Global.h"
#include"Thread.h"
#include"stl/sidgen.h"
#include"cpu/CPU.h"

class Scheduler;

class ThreadManager {
SINGLETON_H(ThreadManager)

private:
    friend class ThreadStateReady;

    friend class ThreadStateZombie;

    friend class ThreadStateRunning;

    friend class ThreadStateInterruptable;

    friend class ThreadStateUninterruptable;

private:
    lr::sstl::IDGenerator<int> pidGen;
    lr::sstl::Map<int, Thread *> threads;
    lr::sstl::List<Thread *> zombieThreads;
    Scheduler *sched;
    SpinLock lock;
public:
    Thread *CreateThread(ThreadType _Type);

    Thread *CreateChildThread(Thread *_Parent);

    void RemoveThread(int _Pid);

    Thread *GetNextThreadToExecute(CPU *_CPU);

    Thread *GetThreadByPID(int _Pid);

    void SendMessage(IPCMessage &_Msg);

    void SetPriority(Thread *_Thread, int _Priority);

    void ClockNotify(uint64_t _Counter);

    bool IsOrphaned(pid_t _Pgrp);

    void KillProcessGroup(pid_t _Pgrp, int _Sig, int _Priv);

    static void ReadDataFromCurrThread(void *dest, void *src, size_t size) {
        AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(
                *AddressSpaceManager::Instance()->GetKernelAddressSpace(), dest,
                *AddressSpaceManager::Instance()->GetCurrentAddressSpace(), src, size);
    }

    static void WriteDataToCurrThread(void *dest, void *src, size_t size) {
        AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(
                *AddressSpaceManager::Instance()->GetCurrentAddressSpace(), dest,
                *AddressSpaceManager::Instance()->GetKernelAddressSpace(), src,
                size);
    }

    static void TransferDateFromOtherThread(void *_DBuffer, Thread *_SThread,
                                            void *_SBuffer, size_t size) {
        AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(
                *AddressSpaceManager::Instance()->GetCurrentAddressSpace(), _DBuffer,
                *_SThread->GetAddressSpace(), _SBuffer,
                size);
    }
};

