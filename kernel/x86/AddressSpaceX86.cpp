#include"Log.h"
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

#include "AddressSpaceX86.h"


AddressSpaceX86::PageDirEntry* AddressSpaceX86::GetPageDirEntry(addr_t _VirtAddr)
{
	PageDirEntry* entry = &cr3[GetPageDirOffset(_VirtAddr)];
	return entry;
}
AddressSpaceX86::PageTableEntry* AddressSpaceX86::GetPageTableEntry(addr_t _VirtAddr)
{
	PageDirEntry* dirEntry = GetPageDirEntry(_VirtAddr);
	if(dirEntry == nullptr||dirEntry->Present == 0)return nullptr;

	addr_t content = *(addr_t*)dirEntry;
	content&=0xFFFFF000;
	PageTableEntry* ptEntry =
		&((PageTableEntry*)(content))[GetPageTableOffset(_VirtAddr)];

	return ptEntry;
}

void AddressSpaceX86::SetAsCurrentSpace()
{
	__asm__ volatile("movl %0,%%cr3\n\t"::"a"(this->cr3));
}

void AddressSpaceX86::ChangeToCurrentSpaceMode()
{
	__asm__ ("movl %%cr0,%%eax\n\t"
			"bts $31,%%eax\n\t"
			"movl %%eax,%%cr0\n\t"
			"jmp 1f\n\t"
			"1:\n\t"::);
}
void AddressSpaceX86::DisableSpaceMode()
{
	__asm__ ("movl %%cr0,%%eax\n\t"
			"btr $31,%%eax\n\t"
			"movl %%eax,%%cr0\n\t"
			"jmp 1f\n\t"
			"1:\n\t"::);
}

addr_t AddressSpaceX86::GetPhisicalAddress(addr_t _VirtAddr)
{
	PageTableEntry* entry = GetPageTableEntry(_VirtAddr);
	if(entry==nullptr)return 0;

	addr_t content = (entry->PageAddress)<<PAGE_SHIFT;
	return content + GetPageInnerOffset(_VirtAddr);
}

bool AddressSpaceX86::VerifyVirtAddress(addr_t _VirtAddr)
{
	PageTableEntry* entry = GetPageTableEntry(_VirtAddr);
	if(entry==nullptr)return false;
	if(entry->Present == 0)return false;
	return true;
}

void AddressSpaceX86::MapVirtAddrToPhysAddr(addr_t _Phy,addr_t _Virt,int isUser,int isWritable)
{
	PageDirEntry* dir = GetPageDirEntry(_Virt);
	PageTableEntry* table;
	if(!dir->Present)
	{
		dir->PageTableAddress = 
			(addr_t)MemoryManager::Instance()->KernelPageAllocate(1)>>PAGE_SHIFT;
		pageDirUsedCount[GetPageDirOffset(_Virt)] = 0;	
		if(dir->PageTableAddress == 0)
		{
			return;
		}
		dir->Present = 1;
		dir->US = 1;
		dir->RW = 1;
	}
	table = GetPageTableEntry(_Virt);
	if(table->Present)
	{
		--pageDirUsedCount[GetPageDirOffset(_Virt)];	
	}
	*(uint32_t*)table = 0;
	++pageDirUsedCount[GetPageDirOffset(_Virt)];	
	table->PageAddress = _Phy>>PAGE_SHIFT;
	table->Present = 1;
	table->US = isUser;
	table->RW = isWritable;
}

void AddressSpaceX86::UnmapVirtAddr(addr_t _Virt)
{
	//Page Must Existed
	PageDirEntry* dir = GetPageDirEntry(_Virt);
	PageTableEntry* table;
	if(!dir->Present)
	{
		return;
	}
	table = GetPageTableEntry(_Virt);
	if(table->Present)
	{
		--pageDirUsedCount[GetPageDirOffset(_Virt)];	
	}
	*(uint32_t*)table = 0;
}

void AddressSpaceX86::SetPageWritable(addr_t _Virt,bool _Present)
{
	PageTableEntry* table;
	table = GetPageTableEntry(_Virt);
	if(table)table->Present = _Present;
}

bool AddressSpaceX86::IfPageWritable(addr_t _Virt)
{
	PageTableEntry* table;
	table = GetPageTableEntry(_Virt);
	if(table&&table->Present&&table->RW==1)return true;
	else return false;
}

size_t AddressSpaceX86::PageSize()
{
	return PAGE_SIZE;
}

addr_t AddressSpaceX86::GetPageDirAddr()
{
	return (addr_t)cr3;
}

AddressSpace* AddressSpaceX86::GetAddressSpace()
{
	auto cr3 = (PageDirEntry*)MemoryManager::Instance()->KernelPageAllocate(1);
	
	if(!cr3)return nullptr;
	else
	{
		//把APCI部分映射一下,不然PageFault会用到APCI的内存区域,导致递归
		auto res = new AddressSpaceX86(cr3);
		Assert(res);
		const addr_t apci = 0xfee00000;
		res->MapVirtAddrToPhysAddr(apci,apci,0,1);
		return res;
	}
}
AddressSpaceX86::AddressSpaceX86(PageDirEntry* _PDE)
{
	Assert(_PDE);
	cr3 = _PDE;
	memset(&cr3[0],0,sizeof(PageDirEntry)*1024);
	
	this->pageDirUsedCount = (uint16_t*)MemoryManager::Instance()->KernelPageAllocate(1);
	Assert(this->pageDirUsedCount);
}

void AddressSpaceX86::GetPageEntryProperty(addr_t _VirtAddr)
{
	return;
}
