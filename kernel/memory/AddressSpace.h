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
#include"Interrupt.h"

class MemoryAllocator;
class AddressSpace
{
	public:
		virtual void SetAsCurrentSpace() = 0;
		virtual void ChangeToCurrentSpaceMode() = 0;
		virtual addr_t GetPhisicalAddress(addr_t _VirtAddr) = 0;
		virtual bool VerifyVirtAddress(addr_t _VirtAddr) = 0;
		virtual void MapVirtAddrToPhysAddr(addr_t _Phy,addr_t _Virt,int isUser = 1,int isWritable = 1,
			int isCachaDisabled = 0) = 0;
		virtual void UnmapVirtAddr(addr_t _Virt) = 0;
		virtual void SetPageWritable(addr_t _Virt,bool _Present) = 0;
		virtual bool IfPageWritable(addr_t _Virt) = 0;
		virtual size_t PageSize() = 0;
		virtual addr_t GetPageDirAddr() = 0;
		virtual void GetPageEntryProperty(addr_t _VirtAddr) = 0;
		virtual void DisableSpaceMode() = 0;
		AddressSpace(){};
};
