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

class HAL
{
	public:
		static const int IRQBase = 0x40;
		virtual int GetCurrentCPUID() = 0;
		virtual bool Initialize() = 0; 
		virtual const char* Type() = 0;
		virtual void InitBSP() = 0;
		virtual ~HAL(){};
		virtual void EOI() = 0;
		virtual void SetMaskOfIRQ(int _IRQ,bool _Masked) = 0;
		virtual void InitAPs(void (*entry)(),size_t _StackSize) = 0;
		virtual void InitAsAP() = 0;
		virtual void InterruptCPU(int Id,int Irq) = 0;
		virtual void InterruptAllOtherCPU(int Irq) = 0;
		
		static HAL* GetProperHAL();
};
