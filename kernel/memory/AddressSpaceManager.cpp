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
#include"AddressSpaceManager.h"
#include"AddressSpaceX86.h"

using namespace lr::sstl;

SINGLETON_CPP(AddressSpaceManager)
{
	this->kernelSpace = AddressSpaceX86::GetAddressSpace();
	Assert(this->kernelSpace);
}


AddressSpace* AddressSpaceManager::GetKernelAddressSpace()
{
	return this->kernelSpace;
}

AddressSpace* AddressSpaceManager::CreateAddressSpace()
{
	AddressSpace* space = AddressSpaceX86::GetAddressSpace();
	spaces.Insert(MakePair(space->GetPageDirAddr(),space));
	return space;
}
AddressSpace* AddressSpaceManager::GetAddressSpaceByPageDirAddr(addr_t _DirAddr)
{
	auto ite = spaces.Find(_DirAddr);
	if(ite == spaces.End())return nullptr;
	else
		return ite->second;
}
AddressSpace* AddressSpaceManager::GetCurrentAddressSpace()
{
	addr_t pageDirAddr = 0;
	__asm__ volatile("movl %%cr3,%%eax\n\t":"=a"(pageDirAddr):);
	return GetAddressSpaceByPageDirAddr(pageDirAddr);
}

void AddressSpaceManager::CopyDataFromAnotherSpace (
		AddressSpace& _DesSpace,void* _Dest,
		AddressSpace& _SrcSpace,void* _Src
		,size_t _Size)
{
	addr_t src = (addr_t)_Src;
	addr_t des = (addr_t)_Dest;
	addr_t phySrc = _SrcSpace.GetPhisicalAddress(src);
	addr_t phyDes = _DesSpace.GetPhisicalAddress(des);
	size_t srcPageSize = _SrcSpace.PageSize(); 
	size_t desPageSize = _DesSpace.PageSize();


	while(_Size--)
	{
		*(char*)phyDes = *(char*)phySrc;
		++src;
		++des;
		++phySrc;
		++phyDes;
		if(src%srcPageSize == 0)
			phySrc = _SrcSpace.GetPhisicalAddress(src);
		if(des%desPageSize == 0)
			phyDes = _DesSpace.GetPhisicalAddress(des);
	}
}

