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
#include"HAL.h"
class IOAPIC;
struct ACPIRSDPStruct;
struct ACPISDTHeader;
struct ACPIIntrCtrlStructHeader;
struct ACPIMADTable;

class APCI:public HAL
{
	SINGLETON_H(APCI)
	private:
		ACPIRSDPStruct* RSDP;
		IOAPIC* ioapic;
	private:
		ACPIRSDPStruct* FindRSDP();		
		void* FindTable(void* _From,void* _To,const char* _Name,size_t _SumLen);
		ACPISDTHeader* FindEntry(const char* _Sig,int _N = 0);
		ACPIIntrCtrlStructHeader*
			FindIntrCtrlStructInMADT(ACPIMADTable* _Table,int _Type,int _N = 0);
		void DisablePIC();
		void SetToSMPMode();
	public:
		virtual bool Initialize()override;
		virtual const char* Type()override;
		virtual ~HALAPICMul()override;
		virtual void EOI()override;
		virtual int GetCurrentCPUID()override;file:///home/ssj/Project/FreeCROS/OS/kernel/arch/x86/AddressSpace.h
		virtual void SetMaskOfIRQ(int _IRQ,bool _Masked)override;
		virtual void InitAPs(void (*_Entry)(),size_t _StackSize)override;
		virtual void InitAsAP()override;
		virtual void InitBSP()override;
		virtual void InterruptCPU(int _Id,int _Irq)override;
		virtual void InterruptAllOtherCPU(int _Irq)override;
};
