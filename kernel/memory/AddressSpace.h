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

class AddressSpace
{
	public:
		struct PageDirEntry
		{
			uint32_t Present:1;
			uint32_t RW:1;
			uint32_t US:1; //0 is Supervisor
			uint32_t PWT:1;
			uint32_t PCD:1;
			uint32_t A:1;
			uint32_t Ignored:1;
			uint32_t PS:1;	//must be zero
			uint32_t Ignored2:4;
			uint32_t PageTableAddress:20;
			PageDirEntry(){*(uint32_t*)this = 0;}
		};
		struct PageTableEntry
		{
			uint32_t Present:1;
			uint32_t RW:1;
			uint32_t US:1; //0 is Supervisor
			uint32_t PWT:1;
			uint32_t PCD:1;
			uint32_t A:1;
			uint32_t D:1;
			uint32_t PAT:1;	//must be zero
			uint32_t G:1;
			uint32_t Ignored2:3;
			uint32_t PageAddress:20;
			PageTableEntry(){*(uint32_t*)this = 0;}
		};
	private:
		MemoryAllocator*  alloc;
		static const int KernelSpacePageSize = (128*1024*1024)/PAGE_SIZE;
		PageDirEntry* cr3;
		uint16_t* pageDirUsedCount;		//Record the size of the pageTableEntries refered by it.
		static PageTableEntry kernelSpaceEntries[KernelSpacePageSize]__attribute__((aligned(4096))); 

		static bool hasRegisterPageFaultIRQ;
		static int IRQId;

		PageDirEntry* GetPageDirEntry(addr_t _VirtAddr);
		PageTableEntry* GetPageTableEntry(addr_t _VirtAddr);

		static inline int GetPageDirOffset(addr_t _VirtAddr)
		{
			return (_VirtAddr>>22)&0x3ff;
		}
		static inline int GetPageTableOffset(addr_t _VirtAddr)
		{
			return (_VirtAddr>>12)&0x3ff;
		}
		static inline int GetPageInnerOffset(addr_t _VirtAddr)
		{
			return (_VirtAddr)&0xfff;
		}

		static int PageFaultHandler(InterruptParams& params);
	public:
		virtual void SetAsCurrentSpace();
		virtual void ChangeToCurrentSpaceMode();
		virtual addr_t GetPhisicalAddress(addr_t _VirtAddr);
		virtual bool VerifyVirtAddress(addr_t _VirtAddr);
		virtual void MapVirtAddrToPhysAddr(addr_t _Phy,addr_t _Virt,int isUser = 1,int isWritable = 1);
		virtual void UnmapVirtAddr(addr_t _Virt);
		virtual void SetPageWritable(addr_t _Virt,bool _Present);
		virtual bool IfPageWritable(addr_t _Virt);
		virtual size_t PageSize();
		virtual addr_t GetPageDirAddr();
		virtual int RegisterPageFault();
		virtual void GetPageEntryProperty(addr_t _VirtAddr);
		virtual void DisableSpaceMode();
		static void CopyDataFromAnotherSpace
			(AddressSpace& _DesSpace,void* _Dest,
			 AddressSpace& _SrcSpace,void* _Src
			 ,size_t _Size);
		AddressSpace();
};
