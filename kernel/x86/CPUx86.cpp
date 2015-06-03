#include"CPUx86.h"
#include"Interrupt.h"
#include"Log.h"
#include"memory/AddressSpaceManager.h"
#include"thread/ThreadManager.h"
#include <cpu/CPUManager.h>
#include"HAL.h"

extern addr_t stack;
void CPUx86::Run()
{
	if(!allDone)return;
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
		//这个进程事没有pid的.
		newThread = &this->idleThread;
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
		Segment::SetBase(gdt.previousTSS,
				(addr_t)&idleThread.GetCPUState().tss);		
		Segment::SetLimit(gdt.previousTSS,sizeof(idleThread.GetCPUState().tss));
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

	currThread = newThread;

	struct{long a,b;}tmp;
	tmp.a = 0;
	tmp.b = 32;
	__asm__("ljmp *%0\n\t"::"m"(tmp.a));
}

void CPUx86::InitPage()
{
	//Init Page
	AddressSpace* kAS = CPUManager::Instance()->GetKernelAddressSpace();
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

CPUx86::CPUx86(CPU::Type _Type)
{
	Segment::SetBase(gdt.serverLDT,(addr_t)&ldtServer);
	Segment::SetLimit(gdt.serverLDT,sizeof(ldtServer));
	Segment::SetBase(gdt.userLDT,(addr_t)&ldtUser);
	Segment::SetLimit(gdt.userLDT,sizeof(ldtUser));
	this->type = _Type;
	this->manager = CPUManager::Instance();
	
	type = Type::BSP;
	this->InitGDT();
	Interrupt::Instance()->Initialize();
	this->InitPage();
	//Init HAL
	//this->manager->AddCPU(this);//这个是不是不应该有
	this->manager->GetHAL()->InitBSP();
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
