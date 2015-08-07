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

#include "Heap.h"
#include <thread/Thread.h>

#define MIN_UNIT (4<<20)

Heap::Heap():allocator(0x30000000,
					   1<<30,
					   MIN_UNIT,
					   MemoryManager::Instance()->GetKernelInitAllocator(),
					   MemoryZoneType::ORDINARY_PAGE_ALLOC) {}
Heap::Heap(Thread* _Thread):Heap()
{
	Assert(_Thread != nullptr);
	Assert(_Thread->belongTo == nullptr);//如果不是一个进程,就直接报错
	this->space = _Thread->GetAddressSpace().Obj();
}


void* Heap::AllocateLargePage(size_t _Count) {
	return this->allocator.Allocate(_Count * MIN_UNIT, 0);
}

void Heap::Deallocate(void *_Addr) {
	this->allocator.Deallocate(_Addr);
}
