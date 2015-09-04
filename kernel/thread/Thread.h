#pragma once

#include"Global.h"
#include"Threads.h"
#include"memory/AddressSpaceManager.h"
#include"memory/MemoryManager.h"
#include"CPUState.h"
#include"stl/slinkedlist.h"
#include"stl/squeue.h"
#include"stl/smap.h"
#include"IPCMessage.h"
#include"ThreadState.h"
#include"WaitableObj.h"


class CPUState;

class ThreadManager;

class Thread {
public:
    friend class ThreadManager;





    /*********************************************************************/
    //进程的主要组成部件//////////////////////////////////////
private:
    lr::sstl::List<Thread *> children;

    AddressSpace* addressSpace;
    CPUState cpuState;

    lr::Ptr<ThreadState> state;
    static const int MAX_THREAD = 1024;
    uint8_t* childBitMap; //It serves to allocate stack space
    int mapSlot = -1;

    lr::sstl::Map<int, File>* fileTable = nullptr;
    lr::sstl::IDGenerator<int>* fidGen;
public:
    SpinLock threadLock;

    AddressSpace* &GetAddressSpace();

    lr::Ptr<ThreadState> &State();

    void ClockNotify(uint64_t _Counter);

    CPUState &GetCPUState();

    int GetNewFileSlot();

    File *GetFileStruct(int _Fid);

    void RemoveFileStruct(int _Fid);
    /*********************************************************************/








    /*********************************************************************/
    //进程体重要信息相关///////////////////////
private:
    pid_t pid;
    pid_t pgrp, session, leader;
    unsigned short uid, euid, suid;
    unsigned short gid, egid, sgid;
    int priority;
private:
    int tty;
    ThreadType threadType;
    uint32_t alarmCounter = 0;
    uint32_t kernelCounter = 0;
    uint32_t userCounter = 0;
    uint32_t cpuCounter = 8000;

    uint32_t stackAddr = 3 << 30;  //Every thread has its own stack ,3GB Pos
    uint32_t stackSize = 1024 * 8;    //32MB

    addr_t kernelStackAddr = 0;
    size_t kernelStackSize = 4 << PAGE_SHIFT;

public:
    lr::sstl::AString cwd;
    Thread *father = nullptr;
    Thread *belongTo = nullptr; //该线程所属的进程，如果是进程，那么这个就是空的。
    int cpuRunningOn = 0;

    pid_t GetPid();

    uint32_t &CPUCounter();

    uint32_t &KernelCounter();

    uint32_t &UserCounter();

    int &Priority();

    ThreadType Type();

    void IncKernelCounter(uint32_t _Inc);

    void IncUserCounter(uint32_t _Inc);

    pid_t GetPgrp() const {
        return pgrp;
    }

    pid_t GetSession() const {
        return session;
    }

    pid_t GetLeader() const {
        return leader;
    }

    unsigned short GetUid() const {
        return uid;
    }

    unsigned short GetEuid() const {
        return euid;
    }

    unsigned short GetSuid() const {
        return suid;
    }

    unsigned short GetGid() const {
        return gid;
    }

    unsigned short GetEgid() const {
        return egid;
    }

    unsigned short GetSgid() const {
        return sgid;
    }

    int GetPriority() const {
        return priority;
    }

    int GetTty() const {
        return tty;
    }

    void SetTty(int tty) {
        Thread::tty = tty;
    }


    /*********************************************************************/






    /*********************************************************************/
    //信号有关,信号值,信号结构体///////////////////////////////////
private:
    SpinLock sigLock;
    lr::sstl::MultiMap<int, pid_t> sigmap;
    lr::sstl::Map<int, sigaction> sigactions;
    CPUState beforeSignal;
    uint32_t mask;
    addr_t signalKernelStackAddr = 0;
    addr_t signalKernelStackSize = 1 << PAGE_SHIFT;
    bool isSignalProcessFinish = false;
    bool isSignalProcessing = false;
public:
    bool hasSignal();

    void Alarm(uint32_t _Us);

    void ResetAlarm();

    uint32_t GetAlarm();

    addr_t AddSignalHandler(int _Signum, addr_t _Handler, int _Flag);

    bool Kill(pid_t _Source, int _Signum);

    bool RestoreFromSignal();

    bool IsProcessingSignal() { return this->isSignalProcessing; };

    bool PrepareToRun();    //Do some check before running. Handle signal~~
    template<class T>
    void PushUserStack(T val, addr_t &stack_Addr);

    template<class T>
    T PopUserStack();
    /*********************************************************************/






    /*********************************************************************/
    //消息传递相关////////////////////////////////////////////
private:
    //Pair of pid and IPCMessage
    lr::sstl::Map<int, lr::sstl::Queue<IPCMessage>> msgMap;
    SpinLock msgLock;
public:
    WaitableObj waitSelf;
    WaitableObj waitIPCSend;
    WaitableObj waitIPCReceive;

    bool ReceiveMessage(IPCMessage &_Msg);

    bool ExtractMessage
            (int _Pid, IPCMessage &_Msg);

    //Get the ipc msg of one spcific thread
    bool ExtractMessage(IPCMessage &_Msg);
    /*********************************************************************/







    /*********************************************************************/
    //线程堆管理部分。
public:
    //页框管理所用的结构体
    struct PageFrame {

    };
private:
// 	lr::sstl::Map<uint32_t,
    /*********************************************************************/











    /*********************************************************************/
public:
    //线程构造/////////////////////////////////
    Thread &operator=(const Thread &) { return *this; }

    Thread(const Thread &) { }

    //Create a thread with a independent address space=
    Thread(pid_t pid, ThreadType _Type, Thread *_Father = nullptr);

    //Create a thread with a dependent address space
    Thread(Thread &_Thread, int _Pid);

    Thread() { };

    void SetEntry(addr_t _Addr);
    //线程构造/////////////////////////////////
    /*********************************************************************/



};

template<class T>
void Thread::PushUserStack(T val, addr_t &stackAddr) {
    //不应该跨两页
    Assert(sizeof(T) <= PAGE_SIZE);
    AddressSpace *s = this->addressSpace;
    addr_t addrhi = s->GetPhisicalAddress(stackAddr);
    addr_t addrlo = s->GetPhisicalAddress(stackAddr - sizeof(T));
    addr_t addr;
    if ( addrlo == 0 ) {
        void *page = MemoryManager::Instance()
                ->KernelPageAllocate(1);

        addr = (addr_t) page;
        s->MapVirtAddrToPhysAddr((addr_t) page, stackAddr - sizeof(T));
        addrlo = s->GetPhisicalAddress(stackAddr - sizeof(T));
    }
    if ( addrhi == 0 ) {
        void *page = MemoryManager::Instance()
                ->KernelPageAllocate(1);

        addr = (addr_t) page;
        s->MapVirtAddrToPhysAddr((addr_t) page, stackAddr);
        addrhi = s->GetPhisicalAddress(stackAddr);
    }
    //运行这里的时候地址空间可能并非该进程的地址空间,所以我们需要直接写入物理地址.
    //首先判断是否跨页.
    if ((addrlo >> PAGE_SHIFT) != (addrhi >> PAGE_SHIFT)) {
        addr_t objaddr = (addr_t) &val;
        //跨页了.
        //计算低地址的页面需要拷贝的大小.
        size_t lowSize = PAGE_UPPER_ALIGN(addrlo) - addrlo;
        size_t highSize = sizeof(T) - lowSize;
        //拷贝到低地址页
        memmove((void *) addrlo, (void *) objaddr, lowSize);
        //高地址
        memmove((void *) (PAGE_LOWER_ALIGN(addrhi)), (void *) (objaddr + lowSize), highSize);
    }
    else {
        memmove((void *) addrlo, &val, sizeof(T));
    }
    stackAddr -= sizeof(T);
    *(T *) stackAddr = val;
}

template<class T>
T Thread::PopUserStack() {

}
