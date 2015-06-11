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

#include "APCI.h"
#include"Global.h"
#include"string.h"
#include"LocalAPCI.h"
#include"IOAPIC.h"
#define RSDP_SIGNATURE 0x5253443250545232ULL
struct ACPIRSDPStruct 
{
	char signature[8];
	uint64_t checkSum:8;
	uint64_t OEMID:48;
	uint64_t Revision:8;
	uint32_t rsdtAddr;
	uint32_t length;
	uint64_t xsdtAddr;
	uint32_t exCheckSum:8;
	uint32_t reserved:24;
}__attribute__((packed));
struct ACPISDTHeader
{
	char signature[4];
	uint32_t length;
	uint64_t reviesion:8;
	uint64_t checkSum:8;
	uint64_t oemID:48;
	uint64_t oemTbaleID;
	uint32_t oemRevision;
	uint32_t creatorID;
	uint32_t creatorRevision;
} __attribute__((packed));

struct ACPIMADTable
{
	ACPISDTHeader header;
	uint32_t localIntrAddr;
	struct
	{
		uint32_t compatible:1;
		uint32_t reserved:31;
	}flags;
	void* intrCtrlStruct;
};
struct ACPIIntrCtrlStructHeader
{
	uint8_t type;
	uint8_t len;
};
struct ACPIIntrCtrlStructProcessor
{
	ACPIIntrCtrlStructHeader header;
	uint8_t ACPIrocessorID;
	uint8_t APICID;
	uint32_t enabled:1;
	uint32_t reserved:31;
};
struct ACPIIntrCtrlStructIOAPIC
{
	ACPIIntrCtrlStructHeader header;
	uint8_t IOAPIC_ID;
	uint8_t reserved;
	uint32_t address;
	uint32_t globalIntrBase;
};

struct ACPIRSDTStuct
{
	ACPISDTHeader header;
	uint32_t entris;
};
struct ACPIXSDTStuct
{
	ACPISDTHeader header;
	uint64_t entris;
};

SINGLETON_CPP(APCI)
{

}
ACPIRSDPStruct* APCI::FindRSDP()	
{
	ACPIRSDPStruct* res = nullptr;
	addr_t ebdaAddr = *(uint16_t*)0x40E;
	char* ebda = (char*)ebdaAddr;
	const char* Name = "RSD PTR ";


	if((res = (ACPIRSDPStruct*)FindTable(ebda,ebda+1024,Name,20)))
		return res;
	if((res = (ACPIRSDPStruct*)FindTable((char*)0xE0000,(char*)0xFFFFF,Name,20)))
		return res;
	return nullptr;
}
bool APCI::Initialize()
{
	ACPIRSDPStruct* RSDP = this->FindRSDP();
	if(RSDP == nullptr)
	{
		return false;
	}
	this->RSDP = RSDP;
	return true;
}

const char* APCI::Type()
{
	return "APIC Multiple Processor";
}

APCI::~APCI()
{

}

void* APCI::FindTable(void* _From,void* _To,const char* _Name,size_t _SumLen)
{
	char* from = (char*)_From;
	char* to = (char*)_To;
	int len = strlen(_Name);
	for(;from<to;from++)
	{
		if(strncmp(from,_Name,len)==0)
		{
			if(Checksum(from,_SumLen)) //只用检查前20位
			{
				return from;
			}
		}
	}
	return nullptr; 
}
ACPISDTHeader* APCI::FindEntry(const char* _Sig,int _N)
{
	if(RSDP == nullptr)
	{
		this->Initialize();
		if(RSDP == nullptr)return nullptr;
	}

	auto traversal = [&](char* _Entry,uint32_t _Count,uint32_t _Size,const char* _Name)->ACPISDTHeader*
	{
		for(uint32_t i = 0;i<_Count;i++)
		{
			ACPISDTHeader* header = (ACPISDTHeader*)(*(addr_t*)_Entry);
			if(strncmp(_Name,header->signature,4) == 0)
			{
				if(Checksum(header,header->length))
				{
					if(_N-- == 0)return header;
				}
			}
			_Entry += _Size;
		}
		return nullptr;
	};
	ACPIRSDTStuct* rsdt = (ACPIRSDTStuct*)RSDP->rsdtAddr;
	ACPIXSDTStuct* xsdt = (ACPIXSDTStuct*)RSDP->xsdtAddr;

	if(xsdt)
	{
		if(Checksum(xsdt,xsdt->header.length))
		{
			return traversal((char*)&xsdt->entris,
					(xsdt->header.length - sizeof(xsdt->header))/sizeof(xsdt->entris),
					sizeof(xsdt->entris),_Sig); 
		}
	}
	if(rsdt)
	{
		if(Checksum(rsdt,rsdt->header.length))
		{
			return traversal((char*)&rsdt->entris,
					(rsdt->header.length - sizeof(rsdt->header))/sizeof(rsdt->entris),
					sizeof(rsdt->entris),_Sig); 
		}
	}
	return nullptr;
}

void APCI::InitBSP()
{
	ACPIMADTable* madt;
	ACPIIntrCtrlStructIOAPIC* IOAPIC;
	ACPIIntrCtrlStructProcessor* cpu;
	LocalAPIC lapic;
	madt = (ACPIMADTable*)FindEntry("APIC");
	if(madt == nullptr)
	{
		//应该抛出异常
		return;
	}
	IOAPIC = (ACPIIntrCtrlStructIOAPIC*)FindIntrCtrlStructInMADT(madt,1);
	cpu = (ACPIIntrCtrlStructProcessor*)FindIntrCtrlStructInMADT(madt,0);		
	
	if(madt->flags.compatible) //应该禁用8259A
	{
		DisablePIC();
	}
	this->ioapic = new class IOAPIC(IOAPIC->address);	
	ioapic->ChangeGlobalIntrBase(HAL::IRQBase,cpu->APICID);	
	lapic.InitAPCI();
	//Start SMP Mode
	SetToSMPMode();

	for(int i = 0;i<ioapic->GetIRQCount();i++)
	{
		ioapic->Unmask(i);
	}
}
ACPIIntrCtrlStructHeader* APCI::FindIntrCtrlStructInMADT(ACPIMADTable* _Table,int _Type,int _N)
{
	char* step = (char*)&_Table->intrCtrlStruct;	
	for(;(addr_t)step - (addr_t)_Table < _Table->header.length;)
	{
		ACPIIntrCtrlStructHeader* header = (ACPIIntrCtrlStructHeader*) step;
		if(header->type == _Type)
		{
			if(_N-- == 0)
				return header;
		}
		step += header->len;
	}
	return nullptr;
}
void APCI::EOI()
{
	LocalAPIC apic;
	apic.EOI();
}

int APCI::GetCurrentCPUID()
{
	return LocalAPIC::GetLocalAPICID();
}
void APCI::SetMaskOfIRQ(int _IRQ,bool _Masked)
{
	if(_Masked == true)
	{
		this->ioapic->Mask(_IRQ-HAL::IRQBase);	
	}
	else
	{
		this->ioapic->Unmask(_IRQ-HAL::IRQBase);	
	}
}
void APCI::InitAPs(void (*entry)(),size_t _StackSize)
{
	ACPIMADTable* madt;
	ACPIIntrCtrlStructProcessor* cpu;
	LocalAPIC lapic;
	madt = (ACPIMADTable*)FindEntry("APIC");
	if(madt == nullptr)
	{
		//应该抛出异常
		return;
	}
	for(int i = 1;;i++)
	{
		cpu = (ACPIIntrCtrlStructProcessor*)FindIntrCtrlStructInMADT(madt,0,i);
		if(!cpu)break;
		lapic.InitAPs(cpu->APICID,entry,(addr_t)new char[_StackSize],_StackSize);
	}
}
void APCI::InitAsAP()
{
	LocalAPIC lapic;
	lapic.InitAPCI();
}
void APCI::DisablePIC()
{
		//Set the base of the 8259A
		// prevent us from the spurious interrupt:In vmwork it will cause strange error;
		Outb_p(0x11,0x20);
		Outb_p(0x11,0xa0);
		Outb_p(0x90,0x21);//Locate the base to 0x90
		Outb_p(0x98,0xa1);
		Outb_p(0x04,0x21);
		Outb_p(0x02,0xa1);
		Outb_p(0x01,0x21);
		Outb_p(0x01,0xa1);
		
		Outb_p(0xff,0xa1);
		Outb_p(0xff,0x21);
}
void APCI::SetToSMPMode()
{
	Outb_p(0x70,0x22);
	Outb_p(0x01,0x23);
}
void APCI::InterruptCPU(int Id,int Irq)
{
	LocalAPIC lapic;
	lapic.InterruptCPU(Id,Irq);
}
void APCI::InterruptAllOtherCPU(int Irq)
{
	LocalAPIC lapic;
	lapic.InterruptAllOtherCPU(Irq);
}

void APCI::SetClockNextCounter(int _Counter)
{
	LocalAPIC::SetClockNextCounter(_Counter);
}

uint32_t APCI::GetClockCurrCounter()
{
	return LocalAPIC::GetClockCurrCounter();
}

void APCI::SetClockIRQ(irq_t _IrqNum)
{
	LocalAPIC::SetClockIRQ(_IrqNum);
}
