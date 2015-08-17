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
#include <cpu/CPUManager.h>

using namespace lr::sstl;

SINGLETON_CPP(AddressSpaceManager) {
    this->InitKernelAddressSpace();
    Assert(this->kernelSpace);

    //必须保证在该类初始化时,Interrupt已经建立完毕
    CPUManager::Instance()->RegisterIRQ(AddressSpaceManager::PageFaultHandler, 14
    );
}

void AddressSpaceManager::InitKernelAddressSpace() {
    this->kernelSpace = this->CreateAddressSpace();
    Assert(this->kernelSpace);
    auto memSize = MemoryManager::Instance()->MemSize();
    for ( addr_t a = 0; a < PAGE_UPPER_ALIGN((addr_t) memSize); a += PAGE_SIZE) {
        this->kernelSpace->MapVirtAddrToPhysAddr(a, a);
    }
    this->kernelSpace->UnmapVirtAddr(0);
}

AddressSpace *AddressSpaceManager::GetKernelAddressSpace() {
    return this->kernelSpace;
}

AddressSpace *AddressSpaceManager::CreateAddressSpace() {
    AddressSpace *space = AddressSpaceX86::GetAddressSpace();
    //在这里需要把内核空间映射进来,不然内核代码将无法到达
    //会导致一切环任务就出错
    for ( addr_t i = 0; i < MemoryManager::Instance()->MemSize() + 4096; i += PAGE_SIZE) {
        space->MapVirtAddrToPhysAddr(i, i, 0);//0为位内核模式
    }
    spaces.Insert(MakePair(space->GetPageDirAddr(), space));
    space->UnmapVirtAddr(0);
    return space;
}

AddressSpace *AddressSpaceManager::GetAddressSpaceByPageDirAddr(addr_t _DirAddr) {
    auto ite = spaces.Find(_DirAddr);
    if ( ite == spaces.End())return nullptr;
    else
        return ite->second;
}

AddressSpace *AddressSpaceManager::GetCurrentAddressSpace() {
    addr_t pageDirAddr = 0;
    __asm__ volatile("movl %%cr3,%%eax\n\t":"=a"(pageDirAddr):);
    return GetAddressSpaceByPageDirAddr(pageDirAddr);
}

void AddressSpaceManager::CopyDataFromAnotherSpace(
        AddressSpace &_DesSpace, void *_Dest,
        AddressSpace &_SrcSpace, void *_Src, size_t _Size) {
    addr_t src = (addr_t) _Src;
    addr_t des = (addr_t) _Dest;
    addr_t phySrc = _SrcSpace.GetPhisicalAddress(src);
    addr_t phyDes = _DesSpace.GetPhisicalAddress(des);
    size_t srcPageSize = _SrcSpace.PageSize();
    size_t desPageSize = _DesSpace.PageSize();


    while ( _Size-- ) {
        *(char *) phyDes = *(char *) phySrc;
        ++src;
        ++des;
        ++phySrc;
        ++phyDes;
        if ( src % srcPageSize == 0 )
            phySrc = _SrcSpace.GetPhisicalAddress(src);
        if ( des % desPageSize == 0 )
            phyDes = _DesSpace.GetPhisicalAddress(des);
    }
}

int AddressSpaceManager::PageFaultHandler(InterruptParams &_Params) {
    addr_t address;
    addr_t cr3;
    __asm__("movl %%cr2,%%eax\n\t":"=a"(address):);
    __asm__("movl %%cr3,%%eax\n\t":"=a"(cr3):);
    AddressSpace *kas = AddressSpaceManager::Instance()->GetKernelAddressSpace();
    AddressSpace *space = AddressSpaceManager::Instance()->GetAddressSpaceByPageDirAddr(cr3);

    Assert(address >= PAGE_SIZE);

    if ( kas == space ) //Kernel Space
    {
        kas->MapVirtAddrToPhysAddr(address, address, 0, 1);
    }
    else //User Space
    {
        if ( address < 0x10000000 || address > 0xf0000000 ) //这个空间以下的应用程序都不能使用
        {
            space->MapVirtAddrToPhysAddr(address, address, 1, 1);
        }
        else {
            //应该是发送消息给Server,这里先这么实现把.
            void *page = MemoryManager::Instance()->
                    KernelPageAllocate(1);
            space->MapVirtAddrToPhysAddr((addr_t) page, address);

        }
    }
    return true;
}

