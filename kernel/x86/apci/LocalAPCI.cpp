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

#include "LocalAPCI.h"
#include"cpuid.h"
#include"stdlib.h"
#include"string.h"
#include"Clock.h"

#define IA32_APIC_BASE_MSR 0x1B
#define IA32_APIC_BASE_MSR_BSP 0x100 // Processor is a BSP
#define IA32_APIC_BASE_MSR_ENABLE 0x800

#define APIC_APICID 0x20
#define APIC_APICVER 0x30
#define APIC_TASKPRIOR 0x80
#define APIC_EOI 0x0B0
#define APIC_LDR 0x0D0
#define APIC_DFR 0x0E0
#define APIC_SPURIOUS 0x0F0
#define APIC_ESR 0x280
#define APIC_ICRL 0x300
#define APIC_ICRH 0x310
#define APIC_LVT_TMR 0x320
#define APIC_LVT_PERF 0x340
#define APIC_LVT_LINT0 0x350
#define APIC_LVT_LINT1 0x360
#define APIC_LVT_ERR 0x370
#define APIC_TMRINITCNT 0x380
#define APIC_TMRCURRCNT 0x390
#define APIC_TMRDIV	0x3E0
#define APIC_LAST	0x38F
#define APIC_DISABLE 0x10000
#define APIC_SW_ENABLE 0x100
#define APIC_CPUFOCUS 0x200
#define APIC_NMI (4<<8)
#define TMR_PERIODIC 0x20000
#define TMR_BASEDIV	(1<<20)
	 

const uint32_t CPUID_FLAG_MSR = 1 << 5;
volatile char* LocalAPIC::baseAddr = (char*)0xFEE00000; 

union IPIMessage
{
	struct
	{
		uint32_t vecotr:8;
		uint32_t deliMode:3;
		uint32_t destMode:1;
		uint32_t deliStatus:1;
		uint32_t reserved:1;
		uint32_t level:1;
		uint32_t triggerMode:1;
		uint32_t reserved2:2;
		uint32_t shortHand:2;
		uint32_t reserved3:12;
		uint32_t reserved4:24;
		uint32_t destination:8;
	}detail;
	struct
	{
		uint32_t low;
		uint32_t high;
	}val;
};

bool LocalAPIC::CPUHasMSR()
{
	uint32_t a, d,c,b; // eax, edx
	__get_cpuid(1, &a,&b,&c,&d);

	return d & CPUID_FLAG_MSR;
}

void LocalAPIC::CPUGetMSR(uint32_t msr, uint32_t *lo, uint32_t *hi)
{
	asm volatile("rdmsr" : "=a"(*lo), "=d"(*hi) : "c"(msr));
}

void LocalAPIC::CPUSetMSR(uint32_t msr, uint32_t lo, uint32_t hi)
{
	asm volatile("wrmsr" : : "a"(lo), "d"(hi), "c"(msr));
}
void LocalAPIC::WriteRegister(int _Offset,uint32_t _Val)
{
	*(uint32_t*)(baseAddr+_Offset) = _Val; 
}

uint32_t LocalAPIC::ReadRegister(int _Offset)
{
	return *(uint32_t*)(baseAddr+_Offset);
}

void LocalAPIC::SetAPICBaseReg(uint64_t _Addr)
{
	uint32_t edx = 0;
	uint32_t eax = (_Addr & 0xfffff800);
	edx = (_Addr >> 32) & 0x0f;

	CPUSetMSR(IA32_APIC_BASE_MSR, eax, edx);
}

uint64_t LocalAPIC::GetAPICBaseReg()
{
	uint32_t eax, edx;
	uint64_t res;
	CPUGetMSR(IA32_APIC_BASE_MSR, &eax, &edx);

	res = edx&0x0f;
	res<<=32;
	res = eax&0xfffff000;

	return res;
}


void LocalAPIC::EOI()
{
	WriteRegister(APIC_EOI,0);
}
int LocalAPIC::GetLocalAPICID()
{
	return ReadRegister(APIC_APICID)>>24;
}
void LocalAPIC::SendIPI(IPIMessage* msg)
{
	IPIMessage tmp;
	tmp.detail.deliStatus = 1;
	WriteRegister(APIC_ICRH,msg->val.high);
	WriteRegister(APIC_ICRL,msg->val.low);
}

void LocalAPIC::InitAPs(int APICID,void (*entry)(),addr_t _StackAddr,size_t _StackSize)
{
	extern char aps_boot_code_start;
	extern char aps_boot_code_end;
	extern char aps_boot_data;
	struct BootData
	{
		uint32_t main_entry;
		uint32_t stack_addr;
		uint32_t stack_size;
		uint32_t flag;//上一个AP初始化好后会将这一位置为0
		uint32_t flag_absolute_addr;
	}*boot_data = (BootData*)&aps_boot_data;
	Assert(boot_data);
	Clock* c = Clock::Instance();
	IPIMessage msg;

	boot_data->main_entry = (uint32_t)entry;
	boot_data->stack_addr = _StackAddr + _StackSize -4;
	boot_data->stack_size = _StackSize;
	boot_data->flag_absolute_addr = (uint32_t)&boot_data->flag;
	while(boot_data->flag == 1);
	boot_data->flag = 1;

	memmove((void*)0x65000,&aps_boot_code_start,&aps_boot_code_end-&aps_boot_code_start+1);

	msg.val.high = 0;
	msg.val.low = 0;
	msg.detail.deliMode = 0b101; //INIT
	msg.detail.destination = APICID;
	SendIPI(&msg);
	
	c->KernelWait(10*1000);
	msg.detail.vecotr = 0x65;
	msg.detail.deliMode = 0b110; //Starup INIT
	SendIPI(&msg);

// 	c->KernelWait(200);
// 	SendIPI(&msg);
}
bool LocalAPIC::InitAPCI()
{
	if(!CPUHasMSR())
	{
		return false;
	}

	//initialize LAPIC to a well known state
	ResetLAPIC();
	EnableLAPIC();

	//Vector:0x20,Notmasked Level,No IRR,High active,Idle,ExtINT;0b01000011100000000
	WriteRegister(APIC_LVT_LINT1,0b01000011100000000|0x20);
	WriteRegister(APIC_LVT_LINT0,0b01000011100000000|0x20);


	//Init APIC Timer
	WriteRegister(APIC_LVT_TMR,32);
	//Divided by 3
	WriteRegister(APIC_TMRDIV,0x3);
	
	uint32_t tmp = ReadRegister(APIC_TMRCURRCNT);

	return true;
}
void LocalAPIC::ResetLAPIC()
{
	//initialize LAPIC to a well known state
	WriteRegister(APIC_DFR, 0xFFFFFFFF);
	WriteRegister(APIC_LDR, (ReadRegister(APIC_LDR)&0x00FFFFFF)|1);
	WriteRegister(APIC_LVT_TMR, APIC_DISABLE);
	WriteRegister(APIC_LVT_PERF, APIC_NMI);
	WriteRegister(APIC_LVT_LINT0, APIC_DISABLE);
	WriteRegister(APIC_LVT_LINT1, APIC_DISABLE);
	WriteRegister(APIC_TASKPRIOR, 0);

}
void LocalAPIC::EnableLAPIC()
{
	//Enable It
	SetAPICBaseReg(GetAPICBaseReg()| IA32_APIC_BASE_MSR_ENABLE);
	WriteRegister(APIC_SPURIOUS,39+APIC_SW_ENABLE); 

}
void LocalAPIC::InterruptCPU(int APICID,int Irq)
{
	IPIMessage msg;

	msg.val.high = 0;
	msg.val.low = 0;
	msg.detail.deliMode = 0b000; //Fixed
	msg.detail.destination = APICID;
	msg.detail.vecotr = Irq;
	SendIPI(&msg);
}
void LocalAPIC::InterruptAllOtherCPU(int Irq)
{
	IPIMessage msg;

	msg.val.high = 0;
	msg.val.low = 0;
	msg.detail.deliMode = 0b000; //Fixed
	msg.detail.shortHand = 0b11; //Excluding itself;	
	msg.detail.vecotr = Irq;
	SendIPI(&msg);
}

void LocalAPIC::SetClockNextCounter(int _Counter)
{
	WriteRegister(APIC_TMRINITCNT,_Counter);
}
uint32_t LocalAPIC::GetClockCurrCounter()
{
	//Counter
	return ReadRegister(APIC_TMRINITCNT);
}
void LocalAPIC::SetClockIRQ(irq_t _IrqNum)
{
	WriteRegister(APIC_LVT_TMR,_IrqNum | (0b01 << 17));
}