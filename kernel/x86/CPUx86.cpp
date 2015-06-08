#include"CPUx86.h"
#include"Interrupt.h"
#include"Log.h"
#include"memory/AddressSpaceManager.h"
#include"thread/ThreadManager.h"
#include <cpu/CPUManager.h>
#include"syscall/SystemCallEntry.h"
#include"HAL.h"
#include "Clock.h"

void idle()
{
	int a = 0;
	for(;;)
	{
	}
}
extern addr_t stack;
void CPUx86::Run()
{
	if(!allDone)return;
	CPUState::TSSStruct tmptss;
	Thread* newThread;
	ThreadManager* tman = ThreadManager::Instance();
	//判断时间片是否用尽
	this->startCounter = CPUManager::Instance()->GetClockCounter();
	if(this->startCounter<=this->endCounter)
	{
		return;
	}
	newThread = tman->GetNextThreadToExecute(this);
	if(newThread == nullptr)
	{
		//没有进程可以运行,我们就运行Idle进程,即idleThread
		if(this->idleThread == nullptr)
		{
			return;
		}
		this->idleThread->State()->ToReady(this->idleThread);
		newThread = this->idleThread;
	}
	//必须有这个判断,防止
	//不断切换到同一个进程会导致栈很快满了
	if(currThread == newThread)
	{
		return;
	}
	//Preparing for switch the thread;
	//Set the TSS
	if(currThread != nullptr)
	{
		Segment::SetBase(gdt.previousTSS,
				Segment::GetBase(gdt.currentTSS));	
	}
	else
	{
		//Segment::SetBase(gdt.previousTSS,
				//(addr_t)&idleThread->GetCPUState().tss);		
		//Segment::SetLimit(gdt.previousTSS,sizeof(idleThread->GetCPUState().tss));
		Segment::SetBase(gdt.previousTSS,(addr_t)&tmptss);
		Segment::SetLimit(gdt.previousTSS,sizeof(tmptss));
	}
	Segment::SetBase(gdt.currentTSS,
			(addr_t)&(newThread->GetCPUState().tss));	
	Segment::SetLimit(gdt.currentTSS,sizeof(newThread->GetCPUState().tss));

	gdt.previousTSS.type = 0b1001;  //Clear the busy flag
	gdt.currentTSS.type = 0b1001;  //Clear the busy flag

	__asm__("ltr %%ax"::"a"(24));

	if(currThread&&currThread->State()->Type() == RUNNING)
	{
		currThread->State()->ToReady(currThread);
	}
	newThread->State()->ToRun(newThread); //Change the state of the thread
	endCounter = startCounter + newThread->CPUCounter();

	this->SaveFPU(currThread);
	currThread = newThread;

	auto eip = currThread->GetCPUState().tss.eip;
// 	LOG("Next task eip: 0x%x\n",eip);
	if(eip< 0x100000)for(;;);
	struct{long a,b;}tmp;
	tmp.a = 0;
	tmp.b = 32;
	__asm__(
		"ljmp *%0\n\t"
	::"m"(tmp.a));
}
void CPUx86::SaveFPU(Thread* _Thread)
{
	__asm__(
		"clts\n\t"
		"fnsave %0\n\t"
		::"m"(_Thread->GetCPUState().tss.i387)
	);
}
void CPUx86::LoadFPU()
{
	Thread* currThread = CPUManager::
		Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Assert(currThread);
	__asm__(
		"clts\n\t"
		"frstor %0\n\t"
		::"m"(currThread->GetCPUState().tss.i387)
	);
}
void CPUx86::InitFPU()
{
	__asm__("fninit\n\t clts\n\t"::);
	CPUManager::Instance()->RegisterIRQ((IRQHandler)CPUx86::LoadFPU,7);
}

void CPUx86::InitPage()
{
	//Init Page
	AddressSpace* kAS = AddressSpaceManager::Instance()->GetKernelAddressSpace();
	kAS->SetAsCurrentSpace();
	kAS->ChangeToCurrentSpaceMode();
}
void CPUx86::InitGDT()
{
	Segment::SetBase(this->gdt.serverLDT,(addr_t)&ldtServer);
	Segment::SetLimit(this->gdt.serverLDT,sizeof(ldtServer));
	Segment::SetBase(this->gdt.userLDT,(addr_t)&ldtUser);
	Segment::SetLimit(this->gdt.userLDT,sizeof(ldtUser));
	Interrupt* intr = Interrupt::Instance();
	intr->Initialize();	
	this->stackAddr = stack; 
	struct
	{
		uint16_t limit;
		uint32_t base;
	}__attribute__((packed))tmp;
	tmp.limit = sizeof(this->gdt);
	tmp.base = (uint32_t)&this->gdt;
	__asm__("lgdt (%0)\n\t"::"a"(&tmp));	
	__asm__("jmpl %0,$g\n\t"
			"g:"::"i"(0x8));
	__asm__("movw %0,%%ds\n\t"
			"movw %0,%%es\n\t"
			"movw %0,%%fs\n\t"
			"movw %0,%%gs\n\t"
			"movw %0,%%ss\n\t"
			::"a"(16));
}
void CPUx86::InitSysCall()
{
	SystemCallEntry::Instance();
}

CPUx86::CPUx86(CPU::Type _Type)
{
	Segment::SetBase(gdt.serverLDT,(addr_t)&ldtServer);
	Segment::SetLimit(gdt.serverLDT,sizeof(ldtServer));
	Segment::SetBase(gdt.userLDT,(addr_t)&ldtUser);
	Segment::SetLimit(gdt.userLDT,sizeof(ldtUser));
	this->type = _Type;
	this->manager = CPUManager::Instance();
	
	type = _Type;
	this->InitGDT();
	Interrupt::Instance()->Initialize();
	
	//现在中断可以用了,InitPage里会调用Interrupt
	this->InitPage();
	this->InitFPU();
	//Init HAL
	//this->manager->AddCPU(this);//这个是不是不应该有
	if(_Type == BSP)
	{
		this->manager->GetHAL()->InitBSP();
	}
	else
	{
		this->manager->GetHAL()->InitAsAP();
	}
	this->InitSysCall();
	this->SetID(this->manager->GetHAL()->GetCurrentCPUID());
	Interrupt::Sti();
}
Thread* CPUx86::GetCurrThreadRunning()
{
	return currThread;
}
CPU::Type CPUx86::GetType()
{
	return type;	
}
void CPUx86::StartService() //表明CPU可以接受线程了
{
	this->allDone = true;
}
void CPUx86::ExhaustCurrThread()
{
	endCounter = 0;
}

CPUx86::~CPUx86()
{

}

void CPUx86::SetIdleThread(Thread* _Thread)
{
	this->idleThread = _Thread;
}
