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

#include "IOAPIC.h"

#define SET_CUR_REG(reg) \
	struct\
{\
	uint32_t addr:8;\
	uint32_t reserved:24;\
}tmp;\
tmp.addr = _Reg;\
tmp.reserved = 0;\
*(uint32_t*)addr = *(uint32_t*)&tmp;

union RegLow
{
	struct
	{
		uint32_t intVec:8;
		uint32_t deliMode:3;
		uint32_t destMode:1;
		uint32_t deliStatus:1;
		uint32_t pinPolar:1;
		uint32_t remoteIRR:1;
		uint32_t triggerMode:1;
		uint32_t mask:1;
		uint32_t reserved:15;
	}Reg;
	uint32_t i;
};
struct RegHigh
{
	struct
	{
		uint32_t reserved:24;
		uint32_t destField:8;
	}Reg;
	uint32_t i;
};

void IOAPIC::WriteToReg(int _Reg,uint32_t _Data)
{
	SET_CUR_REG(_Reg);
	volatile uint32_t* res = (uint32_t*)(addr+0x10);
	*res = _Data;
}
uint32_t IOAPIC::ReadReg(int _Reg)
{
	SET_CUR_REG(_Reg);
	volatile uint32_t* res = (uint32_t*)(addr+0x10);
	return *res;
}
IOAPIC::IOAPIC(addr_t _Addr)
{
	this->addr = _Addr;
	count = 0;
	base = 0;
}

void IOAPIC::ChangeGlobalIntrBase(int _Base,int _LAPIC)
{
	base = _Base;
	RegLow low;
	RegHigh high;
	count = (this->ReadReg(0x1)>>16)&0xff;
	for(int i = 0;i<count;i++)
	{
		low.Reg.intVec = _Base++;	
		low.Reg.deliMode = 0; //Fixed
		low.Reg.destMode = 0;
		low.Reg.pinPolar = 0;//high active
		low.Reg.triggerMode = 0;//edge sensitive
		low.Reg.mask = 1;//masked
		low.Reg.reserved = 0;
		high.Reg.reserved = 0;
		high.Reg.destField = _LAPIC; 
		this->WriteToReg(0x10 + 2*i,low.i);
		this->WriteToReg(0x10 + 2*i +1,high.i);
	}
}
void IOAPIC::Unmask(int i)
{
	volatile RegLow low;
	low.i = ReadReg(0x10 + 2*i);
	low.Reg.mask = 0;
	WriteToReg(0x10+2*i,low.i);
}
void IOAPIC::Mask(int i)
{
	volatile RegLow low;
	low.i = ReadReg(0x10 + 2*i);
	low.Reg.mask = 1;
	WriteToReg(0x10+2*i,low.i);

}
int IOAPIC::GetIRQCount()
{
	if(count == 0)
	{
		uint32_t maxEntries = (this->ReadReg(0x1)>>16)&0xff;
		count = maxEntries;
	}
	return count;
}
