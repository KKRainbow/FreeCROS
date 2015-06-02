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

#include"MemoryManager.h"
#include"Multiboot.h"
#include"MemoryBuddyAllocator.h"
#include"MemoryListAllocator.h"

SINGLETON_CPP(MemoryManager)
{
	Assert(this->InitInitAllocator());
	Assert(this->MoveModulesToSafe());
	Assert(this->ArrangeMemoryLayout());
}

bool MemoryManager::InitInitAllocator()
{
	//初始化initzone,这个区域保证不会被其他关键东西占用
	this->kernelInitAllocator = new(this->initAllocator) MemoryListAllocator(
		(addr_t)&kernelObjInitZoneStart
		,(addr_t)&kernelObjInitZoneEnd - (addr_t)&kernelObjInitZoneStart
		,MemoryZoneType::KERNEL_ARBITRARY_ALLOC
	);
	
	if(!this->kernelInitAllocator->Initialize())
	{
		return false;
	}
	
}

bool MemoryManager::ArrangeMemoryLayout()
{
//根据multiboot里的memoryinfo提供的信息,布局内存	
	const auto& mbInfo = globalMultiboot.GetMultibootInfo();
	
	//这个必须有= =,至于没有的情况,以后再想怎么做
	if(!globalMultiboot.TestFlagMmap())
	{
		return false;
	}

	unsigned long mmlen = mbInfo.mmapLength;
	unsigned long maddr = mbInfo.mmapAddress;
	MultibootMemoryMap* mmap = reinterpret_cast<MultibootMemoryMap*>(maddr);

	//这里为什么是小于号?.
	//这里的主要任务是看内存有多大,不在乎具体的type值
	uint32_t memHighLimit = 0;
	for(int i = 1;maddr<(unsigned long)mbInfo.mmapAddress+mmlen;i++)
	{
		if(mmap->type == 1)
		{
			void* addr = reinterpret_cast<void*>(mmap->baseAddressLow);
			uint32_t tmplen = mmap->baseAddressLow + mmap->lengthLow;
			memHighLimit = tmplen>memHighLimit?tmplen:memHighLimit;
		}
		maddr += mmap->size + sizeof(mmap->size);
		mmap = reinterpret_cast<MultibootMemoryMap*>(maddr);
	}
	
	//判断是否满足最小内存需求(32MB)
	if(memHighLimit <= (32<<20))
	{
		return false;
	}
	
	
	//Page和Object的比例随便写的0 0,这个比例以后慢慢调
	addr_t objStart = (addr_t)&kernelEnd;
	addr_t pageEnd = (addr_t)memHighLimit;
	
	addr_t objEnd = (pageEnd - objStart)/6 + objStart;
	addr_t pageStart = objEnd;
	
	this->kernelPageAllocator = new
	(this->kernelInitAllocator->Allocate(sizeof(MemoryBuddyAllocator)))
		MemoryBuddyAllocator(pageStart,pageEnd-pageStart,PAGE_SIZE
			,this->kernelInitAllocator
			,MemoryZoneType::KERNEL_ARBITRARY_ALLOC
		);
	this->userObjectAllocator = new
		(this->kernelInitAllocator->Allocate(sizeof(MemoryListAllocator)))
		MemoryListAllocator(objStart,objEnd - objStart
			,MemoryZoneType::KERNEL_ARBITRARY_ALLOC
		);
	
	if(!this->kernelPageAllocator || !this->userObjectAllocator)
	{
		return false;
	}
	
	if(!this->kernelPageAllocator->Initialize())
	{
		return false;
	}
	
	if(!this->userObjectAllocator->Initialize())
	{
		return false;
	}
	
	return true;
}

bool MemoryManager::MoveModulesToSafe()
{
	//把所有的Module都移到initZone来,防止被其他东西覆盖
	
	if(this->kernelInitAllocator == nullptr)return false;
	
	const auto& mbInfo = globalMultiboot.GetMultibootInfo();
	
	MultibootModule* mod = 
		(MultibootModule*)globalMultiboot.GetMultibootInfo().modsAddress;
	this->moduleCount = (int16_t)globalMultiboot.GetMultibootInfo().modsCount;
	this->modules = this->kernelInitAllocator->Allocate(sizeof(Module) * this->moduleCount);
	for(int i = 0;i< this->moduleCount;i++)
	{
		this->modules[i].size = mod[i].modEnd-mod[i].modStart;
		this->modules[i].addr = this->kernelInitAllocator->Allocate(
			this->modules[i].size);
		if(this->modules[i].addr == 0)
		{
			return false;
		}
		memmove((void*)this->modules[i].addr,
			(void*)mod[i].modStart,
			this->modules[i].size);
		mod[i].modStart = this->modules[i].addr;
		mod[i].modEnd = this->modules[i].addr + this->modules[i].size;
	}

	return true;
}

MemoryAllocator* MemoryManager::OperatorNewCallback(size_t _Size)
{
	return this->kernelInitAllocator;
}

MemoryAllocator* MemoryManager::OperatorDeleteCallback(void* _Ptr)
{
	return this->kernelInitAllocator;
}