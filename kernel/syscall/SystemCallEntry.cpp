#include"SystemCallEntry.h"
#include"Interrupt.h"
#include"SystemCalls.h"


#define ADD_SYSTEM_CALL(name) \
	this->callMap[SysCall##name::GetCallNum()] = new SysCall##name()
SINGLETON_CPP(SystemCallEntry)
{
	ADD_SYSTEM_CALL(CreateThread);
	ADD_SYSTEM_CALL(Log);
	ADD_SYSTEM_CALL(SendMessageTo);
	ADD_SYSTEM_CALL(ReceiveAll);
	ADD_SYSTEM_CALL(ReceiveFrom);
	ADD_SYSTEM_CALL(ReadDataFromThread);
	ADD_SYSTEM_CALL(WriteToPhisicalAddr);
	ADD_SYSTEM_CALL(RegisterChrDev);
	ADD_SYSTEM_CALL(Open);
	ADD_SYSTEM_CALL(Read);
	Interrupt::Instance()->RegisterIRQ(Call,SYSCALL_IRQ_NUM);	
	Interrupt::Instance()->SetDPLOfIRQ(SYSCALL_IRQ_NUM,1);
}
int SystemCallEntry::Call(InterruptParams& params)
{
	Interrupt::Instance()->Cli();
	auto ins = SystemCallEntry::Instance();
	auto ite = ins->callMap.Find(params.eax);
	if(ite==ins->callMap.End())
	{
		params.eax = -1;
		return -1;
	}
	else
	{
		int res = ite->second->Call(params.ebx
				,params.ecx
				,params.edx
				,params.esi,params);
		params.eax = res;
		return 1;
	}
	Interrupt::Instance()->Sti();
}
	
