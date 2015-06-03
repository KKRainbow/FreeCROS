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
class Thread
{
	public:
	private:
		pid_t pid;
		int priority;
		uint32_t kernelCounter = 0;
		uint32_t userCounter = 0;
		uint32_t cpuCounter = 10000;
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
		
		SpinLock msgLock;
		
		WaitableObj waitSelf;
		WaitableObj waitIPCSend;
		WaitableObj waitIPCReceive;
	private:
		Thread& operator=(const Thread&){return *this;}
		Thread(const Thread&){}
	public:
		//Create a thread with a independent address space=
		Thread(pid_t pid,ThreadType _Type,Thread* _Father = nullptr);		
		//Create a thread with a dependent address space
		Thread(Thread& _Thread,int _Pid);
		
		Thread(){};
		
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
		void PushUserStack(T val,addr_t stack_Addr);
		template<class T>
		T PopUserStack();
};

template<class T>
void Thread::PushUserStack(T val,addr_t stackAddr)
{
	AddressSpace* s = this->addressSpace.Obj();
	addr_t addrhi = s->GetPhisicalAddress(stackAddr);
	addr_t addrlo = s->GetPhisicalAddress(stackAddr - sizeof(T));
	addr_t addr;
	if(addrlo == 0)
	{
		void* page = MemoryManager::Instance()
			->GetKernelPageAllocator()
			->Allocate(PAGE_SIZE,PAGE_SIZE);

		addr = (addr_t)page;
		s->MapVirtAddrToPhysAddr((addr_t)page,stackAddr);
	}
	if(addrhi == 0)
	{
		void* page = MemoryManager::Instance()
			->GetKernelPageAllocator()
			->Allocate(PAGE_SIZE,PAGE_SIZE);

		addr = (addr_t)page;
		s->MapVirtAddrToPhysAddr((addr_t)page,stackAddr);
	}
}
template<class T>
T Thread::PopUserStack()
{

}
