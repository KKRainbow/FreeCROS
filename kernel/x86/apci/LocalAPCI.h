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
union IPIMessage;

class LocalAPIC
{
	private:
		static volatile char* baseAddr;
		static bool CPUHasMSR();
		static void CPUGetMSR(uint32_t _Msr, uint32_t *_Lo, uint32_t *_Hi);
		static void CPUSetMSR(uint32_t _Msr, uint32_t _Lo, uint32_t _Hi);
		static void WriteRegister(int _Offset,uint32_t _Val);
		static uint32_t ReadRegister(int _Offset);
		static void SetAPICBaseReg(uint64_t _Addr);
		static uint64_t GetAPICBaseReg();
		static void SendIPI(IPIMessage* _Msg);
		static void ResetLAPIC();
		static void EnableLAPIC();
	public:
		static bool InitAPCI();
		static void EOI();
		static int GetLocalAPICID();
		static void InitAPs(int _APICID,void (*_Entry)(),addr_t _StackAddr,size_t _StackSize);
		static void InterruptCPU(int _APICID,int _Irq);
		static void InterruptAllOtherCPU(int _Irq);
		
		static void SetClockNextCounter(int _Counter);
		static uint32_t GetClockCurrCounter();
		static void SetClockIRQ(irq_t _IrqNum);
};
