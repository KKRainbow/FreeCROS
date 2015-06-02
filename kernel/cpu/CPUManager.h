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

class AddressSpace;
class Clock;
class HAL;

class CPUManager
{
	SINGLETON_H(CPUManager)
	private:
		AddressSpace* kernelSpace;
		Clock* clock;
		void ClockNotify();
	public:
		lr::sstl::Map<int,CPU*> CPUList; 
		HAL* GetHAL();
		AddressSpace* GetKernelAddressSpace();
		void Initialize();
		CPU* GetCurrentCPU();
		void InitAPs(void (*entry)(),size_t _StackSize);
		void AddCPU(CPU* _CPU);
};

