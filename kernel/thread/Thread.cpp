#include"Thread.h"
#include"string.h"
#include"memory/AddressSpaceManager.h"
#include"Log.h"
#include"IPCMessage.h"
#include <cpu/CPUManager.h>
#include"Threads.h"
//Create Thread::a thread with a independent address space=
Thread::Thread(pid_t pid,ThreadType _Type,Thread* _Father):cpuState(_Type),threadType(_Type)
{
	this->pid = pid;
	if(_Type == ThreadType::KERNEL)
	{
		auto i = 10;
		addressSpace = AddressSpaceManager::Instance()->GetKernelAddressSpace();
		this->stackSize = i*PAGE_SIZE;
		this->stackAddr = (addr_t)MemoryManager::Instance()->
			KernelPageAllocate(i);
			
		LOG("StackAddr: 0x%x",this->stackAddr);
		Assert(this->stackAddr);
	}
	else
	{
		addressSpace = AddressSpaceManager::Instance()->CreateAddressSpace();
	}
	childBitMap = new uint8_t[MAX_THREAD];
	memset(childBitMap.Obj(),0,MAX_THREAD);
	
	cpuState.tss.regs.ebp = 
	cpuState.tss.regs.esp =
	cpuState.tss.esp1 =
	cpuState.tss.esp2 = this->stackAddr + this->stackSize - 4;
	cpuState.tss.cr3 = addressSpace->GetPageDirAddr();

	if(_Father!=nullptr)
	{
		father = _Father;
		_Father->children.PushBack(this);
	}
	this->kernelStackAddr = 
		(addr_t)MemoryManager::Instance()->KernelPageAllocate(kernelStackSize>>PAGE_SHIFT);
	cpuState.tss.esp0 = this->kernelStackAddr + this->kernelStackSize;
	state = ThreadState::GetState(States::UNINTERRUPTABLE);
}

//Create Thread::a thread with a dependent address space
Thread::Thread(Thread& _Thread,int _Pid):cpuState(_Thread.threadType),
	threadType(_Thread.threadType)
{
	pid = _Pid;
	addressSpace = _Thread.addressSpace;	
	childBitMap = _Thread.childBitMap;
	//cpuState = _Thread.cpuState;
	cpuState.tss.cr3 = addressSpace->GetPageDirAddr();
	father = &_Thread;
	_Thread.children.PushBack(this);

	uint8_t* tmp = childBitMap.Obj();
	int i = 0;
	for(;i<MAX_THREAD;i++)
	{
		if(tmp[i] == 0)
		{
			stackAddr = stackAddr - (stackSize*PAGE_SIZE*(i+1));
			tmp[i] = 1;
			cpuState.tss.regs.ebp = 
				cpuState.tss.regs.esp =
				cpuState.tss.esp1 =
				cpuState.tss.esp2 = this->stackAddr + this->stackSize - 4;
			break;
		}
	}
	if(i >= MAX_THREAD)
	{
		state = ThreadState::GetState(States::ZOMBIE);
		return;
	}
	else
	{
		//设置内核栈
		this->kernelStackAddr = 
		(addr_t)MemoryManager::Instance()->KernelPageAllocate(kernelStackSize>>PAGE_SHIFT);
		cpuState.tss.esp0 = this->kernelStackAddr + this->kernelStackSize;
		state = ThreadState::GetState(States::UNINTERRUPTABLE);
		return;
	}
};
uint32_t& Thread::CPUCounter()
{
	return cpuCounter;
}
void Thread::IncKernelCounter(uint32_t _Inc)
{
	kernelCounter+=_Inc;
}
;
void Thread::IncUserCounter(uint32_t _Inc)
{
	userCounter += _Inc;
}
;
uint32_t& Thread::KernelCounter()
{
	return kernelCounter;
}
;
uint32_t& Thread::UserCounter()
{
	return userCounter;
}
;
CPUState& Thread::GetCPUState()
{
	return cpuState;
}
lr::Ptr<AddressSpace>& Thread::GetAddressSpace()
{
	return addressSpace;
}

lr::Ptr<ThreadState>& Thread::State()
{
	return state;
}

//Do some check before running. Handle signal~~
bool Thread::PrepareToRun()
{
	//处理信号
	sigLock.Lock();
	if(this->isSignalProcessFinish == true)
	{
		this->cpuState = this->beforeSignal;
		this->isSignalProcessing = false;
		this->isSignalProcessFinish = false;
		//删除信号内核栈
		Assert(this->signalKernelStackAddr);
		delete (char*)this->signalKernelStackAddr;
		this->signalKernelStackAddr = 0;
	}
	if(this->sigmap.Size() != 0)
	{
		bool flag = false;
		sigaction sigac;
		int num;
		pid_t source;
		auto ite = sigmap.Begin();
		for(;ite!=sigmap.End();ite++)
		{
			num = ite->first;
			if(num >= 32 || (((~mask) & (1<<num)) != 0))
			{
				if(this->sigactions.Find(num) != this->sigactions.End())
				{
					sigac = this->sigactions[num];
				}
				else
				{
					continue;
				}
				flag = true;
				source = ite->second;
				break;
			}
		}
		//有信号..开始处理过程
		if(flag)
		{
			this->sigmap.Erase(ite);
			this->beforeSignal = this->cpuState;
			auto userStackBottom = this->kernelStackAddr;
			InterruptParams* params = (InterruptParams*)
				(this->kernelStackAddr +this->kernelStackSize - sizeof(InterruptParams));
			Assert(params->cs != 0x8);//我们必须确定从这里返回之后不应该是内核
			
			addr_t userEsp = params->userEsp;
			this->PushUserStack(*params,userEsp);
			this->PushUserStack(sigac.sa_handler,userEsp);
			this->PushUserStack(num,userEsp);
			this->PushUserStack(params->eip,userEsp);//方便调试
			
			this->cpuState.tss.cs = params->cs;
			this->cpuState.tss.ss = params->userSS;
			this->cpuState.tss.ds = params->ds;
			this->cpuState.tss.es = params->es;
			this->cpuState.tss.fs = params->fs;
			this->cpuState.tss.gs = params->gs;
			this->cpuState.tss.esp1 = 
				this->cpuState.tss.esp2 = 
				this->cpuState.tss.regs.esp  = userEsp;
			this->cpuState.tss.eip = (addr_t)sigac.lib_sa_handler;
			
			//设置signal的内核栈
			this->signalKernelStackAddr = (addr_t)
				MemoryManager::Instance()->KernelPageAllocate(
					this->signalKernelStackSize >> PAGE_SHIFT
				);
			Assert(this->signalKernelStackSize);
			this->cpuState.tss.esp0 = this->signalKernelStackAddr;
				
			
			this->isSignalProcessing = true;
		}
	}
	sigLock.Unlock();
	return true;
}

bool Thread::ExtractMessage(int _Pid,IPCMessage& _Msg)
	//Get the ipc msg of one spcific thread
{
	msgLock.Lock();
	auto ite = msgMap.Find(_Pid);
	if(ite == msgMap.End())
	{
		msgLock.Unlock();
		return false;
	}
	auto& list = ite->second;
	//Here we need to judge whether we have a valid msg;
	if(list.Size() == 0)
	{
		msgLock.Unlock();
		return false;
	}
	else
	{
		_Msg = list.Top();
		list.Pop();
		msgLock.Unlock();
		return true;
	}
}
bool Thread::ExtractMessage(IPCMessage& _Msg)
{
	msgLock.Lock();
	for(auto& ite : msgMap)
	{
		if(ite.second.Size() != 0)
		{
			_Msg = ite.second.Top();
			ite.second.Pop();
			msgLock.Unlock();
			return true;
		}
	}
	msgLock.Unlock();
	return false;
}
//Get the ipc msg of one spcific thread
pid_t Thread::GetPid()
{
	return pid;
}
void Thread::SetEntry(addr_t _Addr)
{
	cpuState.tss.eip = _Addr; 
}
bool Thread::ReceiveMessage(IPCMessage& _Msg)
{
	msgLock.Lock();
	auto& list = msgMap[_Msg.GetSource()];
	if(list.Size() > 1)
		return false; //Max 10 messages
	list.Push(_Msg);
	msgLock.Unlock();
	return true;
}
int& Thread::Priority()
{
	return this->priority;
}
ThreadType Thread::Type()
{
	return threadType;
}

void Thread::ClockNotify(uint64_t _Counter)
{
	if(this->alarmCounter != 0)
	{
		//TODO: 这里应该改为信号
		if(this->alarmCounter < _Counter)
		{
			//信号版本的实现
			this->Kill(0,SIGALARM);
// 			Message msg;
// 			msg.timeStamp = _Counter;
// 			msg.source = -1;
// 			msg.content[0] = 0x101;
// 			IPCMessage ipc = msg;
// 			this->ReceiveMessage(ipc);
			this->alarmCounter = 0;
		}
	}
}

addr_t Thread::AddSignalHandler(int _Signum,addr_t _Handler,int _Flag)
{
	sigLock.Lock();
	typedef decltype(this->sigactions[_Signum].lib_sa_handler) h_t;
	auto& sigac = this->sigactions[_Signum];
	auto origin = (addr_t)sigac.lib_sa_handler;
	sigac.lib_sa_handler = (h_t)_Handler;
	sigac.sa_flags = _Flag;
	sigLock.Unlock();
	return origin;
}

bool Thread::Kill(pid_t _Source,int _Signum)
{
	sigLock.Lock();
	this->sigmap.Insert(lr::sstl::MakePair(_Signum,_Source));
	sigLock.Unlock();
	return true;
}

bool Thread::RestoreFromSignal()
{
	if(this->isSignalProcessing == false)
	{
		//根本就没有信号被处理!!
		return false;
	}
	this->sigLock.Lock();
	this->isSignalProcessFinish = true;
	this->sigLock.Unlock();
	return true;
}

void Thread::Alarm(uint32_t _Us)
{
	this->alarmCounter = CPUManager::Instance()->GetClockCounter() + _Us;
}