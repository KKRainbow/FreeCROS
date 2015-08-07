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
#include <memory/MemoryBuddyAllocator.h>
class Thread;
class AddressSpace;

class Heap
{
private:
	Heap();
public:
	//必须是进程
	Thread* thread;
	AddressSpace* space;
	MemoryBuddyAllocator allocator;
	Heap(Thread* _Thread);

	//分配连续的页面，堆管理器负责页面映射。0x30000000地址开始为堆区,分配单位为4MB
	//先分配出去，但是并非全部都对应到物理内存。
	//等发生缺页错误时再申请。
	//4M为单位分配的话每个进程不会超过
	void* AllocateLargePage(size_t _Count);

	void Deallocate(void* _Addr);
};
