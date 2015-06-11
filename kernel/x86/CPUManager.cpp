/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include"cpu/CPUManager.h"
#include"memory/AddressSpaceManager.h"
#include "Clock.h"
#include"CPUx86.h"
#include <thread/ThreadManager.h>

SINGLETON_CPP(CPUManager)
{
}

void CPUManager::EOI()
{
	this->hal->EOI();
}
int CPUManager::RegisterIRQ(IRQHandler _Han,IRQNum _Number)
{
	return Interrupt::Instance()->RegisterIRQ(_Han,_Number);
}
bool CPUManager::UnregisterIRQ(int id)
{
	return Interrupt::Instance()->UnregisterIRQ(id);
}
uint64_t CPUManager::GetClockCounter()const
{
	return Clock::Instance()->GetCurrentCounter();
}
uint32_t CPUManager::GetClockPeriod()const
{
	return Clock::Instance()->GetPeriod();
}
void CPUManager::KernelWait(uint32_t _Us)const
{
	Clock::Instance()->KernelWait(_Us);
}

void CPUManager::APsEntryCaller(addr_t _StackAddr,size_t _StackSize)
{
	auto& apsEntry = CPUManager::Instance()->apsEntry;
	auto& apsLock = CPUManager::Instance()->apsLock;
	apsLock.Lock();
	CPU* cpu = new CPUx86(CPU::Type::AP);
	CPUManager::Instance()->AddCPU(cpu);
	
	auto hal = CPUManager::Instance()->GetHAL();
	hal->SetClockIRQ(Clock::CLOCK_IRQ);
	hal->SetClockNextCounter(8000);
	
	apsLock.Unlock();
	//为了防止lock把中断关了,我们要再打开它
	Interrupt::Sti();;
	apsEntry(_StackAddr,_StackSize);
	for(;;);
}
int CPUManager::InitAP(addr_t _Entry,size_t _StackSize)
{
	this->apsEntry = (cpu_entry_t)_Entry;
	this->hal->InitAPs((void (*)())CPUManager::APsEntryCaller,_StackSize);
	return 1;
}

void CPUManager::Initialize()
{
	this->hal = HAL::GetProperHAL();
	Assert(this->hal);
	this->hal->InitBSP();
	
	this->kernelSpace = AddressSpaceManager::Instance()->GetKernelAddressSpace();
	
	auto bsp = new CPUx86(CPU::Type::BSP);
	this->AddCPU(bsp);
	//现在中断可以用了
	
	this->clock = Clock::Instance();
	this->clock->InitPIT();
}
HAL* CPUManager::GetHAL()
{
	return hal;
}
void CPUManager::ClockNotify()
{
	CPU* cpu = this->GetCurrentCPU();	
	if(cpu->GetType() == CPU::Type::BSP) //需要通知其他CPU
	{
// 		ThreadManager::Instance()->ClockNotify(Clock::Instance()->GetCurrentCounter());
// 		GetHAL()->InterruptAllOtherCPU(Clock::CLOCK_IRQ);		
	}
	else
	{
	}
	cpu->Run();//下一轮,CPU由时钟驱动
}

CPU* CPUManager::GetCurrentCPU()
{
	auto ite = CPUList.Find(this->hal->GetCurrentCPUID());
	if(ite == CPUList.End())return nullptr;
	else return ite->second;	
}
int CPUManager::GetCurrentCPUID()
{
	return this->hal->GetCurrentCPUID();
}

void CPUManager::AddCPU(CPU* _CPU)
{
	_CPU->id = this->hal->GetCurrentCPUID();
	CPUList.Insert(lr::sstl::MakePair(_CPU->id,_CPU));
}
