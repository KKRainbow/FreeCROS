#include"SystemCalls.h"
#include"memory/AddressSpaceManager.h"
#include"cpu/CPUManager.h"
#include"thread/ThreadManager.h"
#include"Log.h"
#include"ramdisk/RamDisk.h"

static void ReadDataFromCurrThread(void* dest,void* src,size_t size)
{
	AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(
			*AddressSpaceManager::Instance()->GetKernelAddressSpace(),dest,
			*AddressSpaceManager::Instance()->GetCurrentAddressSpace(),src
			,size);
}

static void WriteDataToCurrThread(void* dest,void* src,size_t size)
{
	AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(
			*AddressSpaceManager::Instance()->GetCurrentAddressSpace(),dest,
			*AddressSpaceManager::Instance()->GetKernelAddressSpace(),src,
			size);
}
static void TransferDateFromOtherThread(void* _DBuffer,Thread* _SThread,
						void* _SBuffer,size_t size)
{
	AddressSpaceManager::Instance()->CopyDataFromAnotherSpace(
			*AddressSpaceManager::Instance()->GetCurrentAddressSpace(),_DBuffer,
			*_SThread->GetAddressSpace(),_SBuffer,
			size);
}
SYSCALL_METHOD_CPP(CreateThread)
{
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	//判断当前Thread是线程还是进程,如果是线程，那么新的线程的父线程理所当然是进程。
	Thread* newThread = ThreadManager::Instance()->CreateChildThread(curr);
	if(newThread->State()->Type() == ZOMBIE)
	{
		return -1;
	}
	else
	{
		newThread->GetCPUState().tss.regs.eax = 0;
		newThread->GetCPUState().tss.eip = _First;
		newThread->State()->ToReady(newThread);
		return newThread->GetPid();
	}
}

SYSCALL_METHOD_CPP(WriteToPhisicalAddr)
{
	ReadDataFromCurrThread((void*)_First,(void*)_Sec,_Third);
	return 1;
}

SYSCALL_METHOD_CPP(ReadFromPhisicalAddr)
{
	WriteDataToCurrThread((void*)_First,(void*)_Sec,_Third);
	return 1;
}

SYSCALL_METHOD_CPP(Log)
{
	char str[500];
	if(_Sec>=500)return -1;
	ReadDataFromCurrThread(str,(void*)_First,_Sec);
	LOG(str,1);	
	return 1;
}

SYSCALL_METHOD_CPP(SendMessageTo)
{
	Message msg;
	ReadDataFromCurrThread(&msg,(void*)_First,sizeof(Message));

	auto curr = CPUManager::Instance()->GetCurrentCPU()
		->GetCurrThreadRunning();
	if(_Sec & SEND_MESSAGE_FLAG_PROXY_PROCESS) {
		auto t = curr->belongTo ? curr->belongTo : curr;
		msg.source = t->GetPid();
	}
	else if (_Sec & SEND_MESSAGE_FLAG_PROXY_FATHER){
		auto t = curr->father ? curr->father : curr;
		msg.source = t->GetPid();
	}
	else
	{
		msg.source = curr->GetPid();
	}
	msg.timeStamp = CPUManager::Instance()->GetClockCounter();
	Thread* des = ThreadManager::Instance()->GetThreadByPID(msg.destination);
	if(des == nullptr)return -1;

	IPCMessage ipc(msg);

	while(!des->ReceiveMessage(ipc))
	{
		des->waitIPCSend.Wait();
	}

	des->waitIPCReceive.Wake();
	return 1;
}

SYSCALL_METHOD_CPP(ReceiveFrom)
{
	IPCMessage ipc;
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	while(!curr->ExtractMessage(_First,ipc))
	{
		curr->waitIPCReceive.Wait();
	}
	curr->waitIPCSend.Wake();
	WriteDataToCurrThread((void*)_Sec,&ipc.msg,sizeof(ipc.msg));	
	return 1;
}

SYSCALL_METHOD_CPP(ReceiveAll)
{
	IPCMessage ipc;
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	while(!curr->ExtractMessage(ipc))
	{
		curr->waitIPCReceive.Wait();	
	}
	curr->waitIPCSend.Wake();
	WriteDataToCurrThread((void*)_First,&ipc.msg,sizeof(ipc.msg));	
	return 1;
}

SYSCALL_METHOD_CPP(ReadDataFromThread)
{
	return 1;
}

static lr::sstl::MultiMap<irq_t,pid_t> irqMap;
static bool isInitialized = false;
static int RegisteredIRQHandler(InterruptParams& params)
{
	auto range = irqMap.EqualRange(params.irqnum);
	auto& b = range.first;
	auto& e = range.second;
	for(;b!=e;)
	{
		pid_t pid = b->second;
		auto t = ThreadManager::Instance()->GetThreadByPID(pid);
		if(t == nullptr)
		{
			auto tmp = b;
			b++;
			irqMap.Erase(tmp);
		}
		else
		{
			t->Kill(0,SIGINT);
			b++;
		}
	}
	return 1;
}
SYSCALL_METHOD_CPP(RegisterIRQ)//irqnum
{
	if(_First > 254)return -1;
	if(isInitialized == false)
	{
		new (&irqMap) decltype(irqMap)();
		isInitialized = true;
	}
	auto id = CPUManager::Instance()->RegisterIRQ((IRQHandler)RegisteredIRQHandler,_First);
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	irqMap.Insert(lr::sstl::MakePair((irq_t)_First,curr->GetPid()));
	return id;
}

SYSCALL_METHOD_CPP(RegisterChrDev) //devname
{
	auto rd = RamDisk::Instance();
	Assert(rd);
	
	char devname[500];
	ReadDataFromCurrThread(devname,(void*)_First,sizeof(devname) - 1);
	devname[500] = 0;
	auto ri = rd->RegisterCharaterDevice(devname);
	return ri->GetID();
}

SYSCALL_METHOD_CPP(Open)//path
{
	auto rd = RamDisk::Instance();
	Message msg;
	char devname[500];
	
	ReadDataFromCurrThread(devname,(void*)_First,sizeof(devname) - 1);
	devname[500] = 0;
	auto ri = rd->GetItemByPath(devname);
	if(ri == nullptr)return -1;

	auto res = ri->Open();
	//如果错误了就不用进行下面的步骤
	if(res < 0)return res;
	//判断打开的设备类型
	switch(ri->GetType())
	{
		case RamDiskItem::Type::CHAR:
			//获取的是设备进程的pid,要想获取最终结果还需ReceiMsg一下
			SysCallReceiveFrom::Invoke((uint32_t)res,(uint32_t)&msg,0,0);
			if(msg.content[0])return ri->GetID();	
			else return -1;
		default:
			break;
	}
	return -1;
}

SYSCALL_METHOD_CPP(Read)//path
{
	auto rd = RamDisk::Instance();
	Message msg;
	
	auto ri = rd->GetItemByID(_First);
	if(ri == nullptr)return -1;
	
	auto res = ri->Read((int8_t*)_Sec,_Third);
	//如果错误了就不用进行下面的步骤
	if(res < 0)return res;
	//判断打开的设备类型
	auto devThread = ThreadManager::Instance()->GetThreadByPID(res);
	if(!devThread)return -1;
	switch(ri->GetType())
	{
		case RamDiskItem::Type::CHAR:
			//获取的是设备进程的pid,要想获取最终结果还需ReceiMsg一下
			SysCallReceiveFrom::Invoke((uint32_t)res,(uint32_t)&msg,0,0);
			//参数: data,size;
			if(msg.content[0] == 0)return -1;
			TransferDateFromOtherThread((void*)_Sec,devThread,
				(void*)msg.content[0],msg.content[1]
			);
// 			LOG("CORE: %d\n",CPUManager::Instance()->GetCurrentCPUID());
			return msg.content[1];
		default:
			break;
	}
	return -1;
}




SYSCALL_METHOD_CPP(Signal) //signum,handler,flag
{
	auto t = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(t);
	return t->AddSignalHandler(_First,_Sec,_Third);
}

SYSCALL_METHOD_CPP(Kill) //pid,signum
{
	auto t = ThreadManager::Instance()->GetThreadByPID(_First);
	if(t == nullptr)return -1;
	
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(curr);
	
	if(t->Kill(curr->GetPid(),_Sec))
		return true;
	return false;
	
}

SYSCALL_METHOD_CPP(SignalRestore)//no params
{
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	if(curr&&curr->RestoreFromSignal())
	{
		CPUManager::Instance()->GetCurrentCPU()->ExhaustCurrThread();
		CPUManager::Instance()->GetCurrentCPU()->Run();
		//这个返回值其实没有意义
		return 1;
	}
	return -1;
}

SYSCALL_METHOD_CPP(Alarm) //time
{
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	curr->Alarm(_First * 1e3);
	return 0;
}


SYSCALL_METHOD_CPP(Pause) 
{
	WaitableObj wait;
	wait.Wait();
	return 0;
}
SYSCALL_METHOD_CPP(Sleep) 
{
	WaitableObj wait;
	wait.Sleep();
	return 0;
}

SYSCALL_METHOD_CPP(WakeUp)
{
	auto t = ThreadManager::Instance()->GetThreadByPID(_First);
	if(!t)return -1;
	if(t->State()->Type() == INTERRUPTABLE)
	{
		t->State()->ToReady(t);
	}
	return 0;
}

SYSCALL_METHOD_CPP(Exit)
{
 	//这里应该回收所有线程占用的资源
	auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	//先这么简单的实现，先实验tty的功能,便于写调试命令,
	//TODO 如果结束的是进程，那么还需要删掉所有的线程。
	ThreadManager::Instance()->RemoveThread(curr->GetPid());
	CPUManager::Instance()->GetCurrentCPU()->ExhaustCurrThread();
	CPUManager::Instance()->GetCurrentCPU()->Run();
}