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
class Thread
{
public:
	friend class ThreadManager;
private:
	pid_t pid;
	int priority;
	uint32_t alarmCounter = 0;
	uint32_t kernelCounter = 0;
	uint32_t userCounter = 0;
	uint32_t cpuCounter = 8000;
	lr::Ptr<AddressSpace> addressSpace; 
	CPUState cpuState;
	lr::sstl::List<Thread*> children;
	Thread* father = nullptr;
	lr::Ptr<ThreadState> state;
	uint32_t stackAddr = 3<<30;  //Every thread has its own stack ,3GB Pos
	uint32_t stackSize = 1024*8;	//32MB
	static const int MAX_THREAD = 1024;
	lr::Ptr<uint8_t> childBitMap; //It serves to allocate stack space
	//Pair of pid and IPCMessage
	lr::sstl::Map<int,lr::sstl::Queue<IPCMessage>> msgMap;
	ThreadType threadType;
	
	addr_t kernelStackAddr = 0;
	size_t kernelStackSize = 4 << PAGE_SHIFT;
	
	SpinLock msgLock;
	SpinLock sigLock;
	
	//信号有关
	//信号值,信号结构体
	lr::sstl::MultiMap<int,pid_t> sigmap;
	lr::sstl::Map<int,sigaction> sigactions;
	CPUState beforeSignal;
	uint32_t mask;
	addr_t signalKernelStackAddr = 0;
	addr_t signalKernelStackSize = 1 << PAGE_SHIFT;
	bool isSignalProcessFinish = false;
	bool isSignalProcessing = false;
public:
	void Alarm(uint32_t _Us);
private:
	///////////////
// private:
public:
	Thread& operator=(const Thread&){return *this;}
	Thread(const Thread&){}
	//Create a thread with a independent address space=
	Thread(pid_t pid,ThreadType _Type,Thread* _Father = nullptr);		
	//Create a thread with a dependent address space
	Thread(Thread& _Thread,int _Pid);
	Thread(){};
	void ClockNotify(uint64_t _Counter);
public:
	WaitableObj waitSelf;
	WaitableObj waitIPCSend;
	WaitableObj waitIPCReceive;
	int cpuRunningOn = 0;
	
	pid_t GetPid();
	uint32_t& CPUCounter();
	uint32_t& KernelCounter();
	uint32_t& UserCounter();
	CPUState& GetCPUState();
	int& Priority();
	ThreadType Type();
	
	void IncKernelCounter(uint32_t _Inc);
	void IncUserCounter(uint32_t _Inc);
	
	lr::Ptr<AddressSpace>& GetAddressSpace();
	lr::Ptr<ThreadState>& State();
	
	void SetEntry(addr_t _Addr);
	
	bool ReceiveMessage(IPCMessage& _Msg);
	bool ExtractMessage
	(int _Pid,IPCMessage& _Msg);//Get the ipc msg of one spcific thread
	bool ExtractMessage(IPCMessage& _Msg);
	
	bool PrepareToRun();	//Do some check before running. Handle signal~~
	
	template<class T>
	void PushUserStack(T val,addr_t& stack_Addr);
	template<class T>
	T PopUserStack();
	
	addr_t AddSignalHandler(int _Signum,addr_t _Handler,int _Flag);
	bool Kill(pid_t _Source,int _Signum);
	bool RestoreFromSignal();
	bool IsProcessingSignal(){return this->isSignalProcessing;};
	
public:
	SpinLock threadLock;
};

template<class T>
void Thread::PushUserStack(T val,addr_t& stackAddr)
{
	//不应该跨两页
	Assert(sizeof(T)<= PAGE_SIZE);
	AddressSpace* s = this->addressSpace.Obj();
	addr_t addrhi = s->GetPhisicalAddress(stackAddr);
	addr_t addrlo = s->GetPhisicalAddress(stackAddr - sizeof(T));
	addr_t addr;
	if(addrlo == 0)
	{
		void* page = MemoryManager::Instance()
		->KernelPageAllocate(1);
		
		addr = (addr_t)page;
		s->MapVirtAddrToPhysAddr((addr_t)page,stackAddr - sizeof(T));
		addrlo = s->GetPhisicalAddress(stackAddr - sizeof(T));
	}
	if(addrhi == 0)
	{
		void* page = MemoryManager::Instance()
		->KernelPageAllocate(1);
		
		addr = (addr_t)page;
		s->MapVirtAddrToPhysAddr((addr_t)page,stackAddr);
		addrhi = s->GetPhisicalAddress(stackAddr);
	}
	//运行这里的时候地址空间可能并非该进程的地址空间,所以我们需要直接写入物理地址.
	//首先判断是否跨页.
	if((addrlo>>PAGE_SHIFT) != (addrhi>>PAGE_SHIFT))
	{
		addr_t objaddr = (addr_t)&val;
		//跨页了.
		//计算低地址的页面需要拷贝的大小.
		size_t lowSize = PAGE_UPPER_ALIGN(addrlo) - addrlo;
		size_t highSize = sizeof(T) - lowSize;
		//拷贝到低地址页
		memmove((void*)addrlo,(void*)objaddr,lowSize);
		//高地址
		memmove((void*)(PAGE_LOWER_ALIGN(addrhi)),(void*)(objaddr+lowSize),highSize);
	}
	else
	{
		memmove((void*)addrlo,&val,sizeof(T));
	}
	stackAddr -= sizeof(T);
	*(T*)stackAddr = val;
}
template<class T>
T Thread::PopUserStack()
{
	
}
