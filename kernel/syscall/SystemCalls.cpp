#include"SystemCalls.h"
#include"memory/AddressSpaceManager.h"
#include"cpu/CPUManager.h"
#include"thread/ThreadManager.h"
#include"Log.h"

static void ReadDataFromCurrThread(void* dest,void* src,size_t size)
{
	AddressSpaceManager::CopyDataFromAnotherSpace(
			*AddressSpaceManager::Instance()->GetKernelAddressSpace(),dest,
			*AddressSpaceManager::Instance()->GetCurrentAddressSpace(),src
			,size);
}

static void WriteDataToCurrThread(void* dest,void* src,size_t size)
{
	AddressSpaceManager::CopyDataFromAnotherSpace(
			*AddressSpaceManager::Instance()->GetCurrentAddressSpace(),dest,
			*AddressSpaceManager::Instance()->GetKernelAddressSpace(),src,
			size);
}
SYSCALL_METHOD_CPP(CreateThread)
{
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
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
	msg.source = CPUManager::Instance()->GetCurrentCPU()
		->GetCurrThreadRunning()->GetPid();
	msg.timeStamp = Clock::Instance()->GetCurrentCounter();

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
	while(!curr->ExtractMessage(_First,ipc))
	{
		curr->waitIPCReceive.Wait();	
	}
	curr->waitIPCSend.Wake();
	WriteDataToCurrThread((void*)_First,&ipc.msg,sizeof(ipc.msg));	
}

SYSCALL_METHOD_CPP(ReceiveAll)
{
	IPCMessage ipc;
	Thread* curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	while(!curr->ExtractMessage(ipc))
	{
		curr->waitIPCReceive.Wait();	
	}
	curr->waitIPCSend.Wake();
	WriteDataToCurrThread((void*)_First,&ipc.msg,sizeof(ipc.msg));	
}

SYSCALL_METHOD_CPP(ReadDataFromThread)
{
	return 1;
}

SYSCALL_METHOD_CPP(RegisterIRQ)
{
	return 1;
}
