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

#include"Global.h"
#include"stl/smap.h"
#include"Interrupt.h"
#include"CPU.h"
#include"SpinLock.h"

class AddressSpace;
class Clock;
class HAL;

typedef void (*cpu_entry_t)(addr_t _StackAddr,size_t _StackSize);
class CPUManager
{
	SINGLETON_H(CPUManager)
	private:
		friend class Clock;
		AddressSpace* kernelSpace;
		Clock* clock;
		void ClockNotify();
		HAL* hal;
		cpu_entry_t apsEntry;
		SpinLock apsLock;
		static void APsEntryCaller(addr_t _StackAddr,size_t _StackSize);
	public:
		lr::sstl::Map<int,CPU*> CPUList; 
		HAL* GetHAL();
		void Initialize();
		CPU* GetCurrentCPU();
		int InitAP(addr_t _Entry,size_t _StackSize);
		void AddCPU(CPU* _CPU);
		void EOI();
		int RegisterIRQ(IRQHandler _Han,IRQNum _Number);
		bool UnregisterIRQ(int _Id);
		uint64_t GetClockCounter()const;
		uint32_t GetClockPeriod()const;
		void KernelWait(uint32_t _Us)const;
};

