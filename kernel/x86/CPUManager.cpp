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

#pragma once
#include"cpu/CPUManager.h"
#include"memory/AddressSpaceManager.h"
#include "Clock.h"
#include"CPUx86.h"

SINGLETON_CPP(CPUManager)
{
	this->hal = HAL::GetProperHAL();
	Assert(this->hal);
	
	this->kernelSpace = AddressSpaceManager::Instance()->CreateAddressSpace();
	this->AddCPU(new CPUx86(CPU::Type::BSP));
	//现在中断可以用了
	
	this->clock = Clock::Instance();
	this->clock->InitPIT();
}

void CPUManager::EOI()
{
	this->hal->EOI();
}
int CPUManager::RegisterIRQ(IRQHandler _Han,IRQNum _Number)
{
	return Interrupt::RegisterIRQ(_Han,_Number);
}
bool CPUManager::UnregisterIRQ(int id)
{
	return Interrupt::UnregisterIRQ(id);
}
uint64_t CPUManager::GetClockCounter()
{
	return Clock::Instance()->GetCurrentCounter();
}
uint32_t CPUManager::GetClockPeriod()
{
	return Clock::Instance()->GetPeriod();
}
void CPUManager::KernelWait(uint32_t _Us)
{
	Clock::Instance()->KernelWait();
}

void CPUManager::InitAPs(cpu_entry_t _Entry,size_t _StackSize)
{
	this->hal->InitAPs(_Entry,_StackSize);
}

void CPUManager::Initialize()
{
	this->clock = Clock::Instance();
}
AddressSpace* CPUManager::GetKernelAddressSpace()
{
	return this->kernelSpace;
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
		GetHAL()->InterruptAllOtherCPU(Clock::CLOCK_IRQ);		
	}
	cpu->Run();//下一轮,CPU由时钟驱动
}

CPU* CPUManager::GetCurrentCPU()
{
	auto ite = CPUList.Find(this->hal->GetCurrentCPUID());
	if(ite == CPUList.End())return nullptr;
	else return ite->second;	
}

void CPUManager::AddCPU(CPU* _CPU)
{
	_CPU->id = this->hal->GetCurrentCPUID();
	CPUList.Insert(lr::sstl::MakePair(_CPU->id,_CPU));
}
