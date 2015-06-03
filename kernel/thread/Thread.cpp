#include"Thread.h"
#include"string.h"
//Create Thread::a thread with a independent address space=
Thread::Thread(pid_t pid,ThreadType _Type,Thread* _Father):cpuState(_Type),threadType(_Type)
{
	this->pid = pid;
	addressSpace = AddressSpaceManager::Instance()->CreateAddressSpace();
	childBitMap = new uint8_t[MAX_THREAD];
	memset(childBitMap.Obj(),0,MAX_THREAD);
	if(_Type != KERNEL)
	{
		cpuState.tss.regs.ebp = 
			cpuState.tss.regs.esp =
			cpuState.tss.esp1 =
			cpuState.tss.esp2 = this->stackAddr + this->stackSize - 4;
	}
	cpuState.tss.cr3 = addressSpace->GetPageDirAddr();

	if(_Father!=nullptr)
	{
		father = _Father;
		_Father->children.PushBack(this);
	}
	state = ThreadState::GetState(States::UNINTERRUPTABLE);
}

//Create Thread::a thread with a dependent address space
Thread::Thread(Thread& _Thread,int _Pid):cpuState(_Thread.cpuState),
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
