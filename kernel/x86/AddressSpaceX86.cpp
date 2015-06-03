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
			((addr_t)alloc->Allocate(PAGE_SIZE,PAGE_SIZE))>>PAGE_SHIFT;
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
	MAGIC_DEBUG;
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
	auto alloc = MemoryManager::Instance()->GetKernelPageAllocator();
	
	auto cr3 = (PageDirEntry*)alloc->Allocate((size_t)PAGE_SIZE,PAGE_SIZE);
	
	if(!alloc || !cr3)return nullptr;
	else
	{
		return new AddressSpaceX86(alloc,cr3);
	}
}
AddressSpaceX86::AddressSpaceX86(MemoryAllocator* _Alloc,PageDirEntry* _PDE)
{
	Assert(_Alloc&&_PDE);
	this->alloc = _Alloc;
	cr3 = _PDE;
	memset(&cr3[0],0,sizeof(PageDirEntry)*1024);
}

void AddressSpaceX86::GetPageEntryProperty(addr_t _VirtAddr)
{
	return;
}
