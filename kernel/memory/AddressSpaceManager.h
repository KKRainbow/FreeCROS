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
#include"AddressSpace.h"
#include"stl/smap.h"

class AddressSpaceManager
{
	SINGLETON_H(AddressSpaceManager)
	private:
		lr::sstl::Map<addr_t,AddressSpace*> spaces;	
		AddressSpace* kernelSpace;
		static int PageFaultHandler(InterruptParams& _Params);
		void InitKernelAddressSpace();
	public:
		AddressSpace* CreateAddressSpace();
		AddressSpace* GetAddressSpaceByPageDirAddr(addr_t _DirAddr);
		AddressSpace* GetCurrentAddressSpace();
		AddressSpace* GetKernelAddressSpace();
		void CopyDataFromAnotherSpace
			(AddressSpace& _DesSpace,void* _Dest,
			 AddressSpace& _SrcSpace,void* _Src
			 ,size_t _Size);
};
