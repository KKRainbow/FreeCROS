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
#include"MemoryListAllocator.h"
#include"stl/slinkedlist.h"

class AddressSpace;
class MemoryAllocator;

class MemoryManager
{
	SINGLETON_H(MemoryManager)
private:
	//Manager初始化时new还不能使用,我们在这个空间初始化第一个Allocator,以此为其他allocator的基础
	uint8_t initAllocator[sizeof(MemoryListAllocator)];
	struct Module{
		addr_t addr;
		size_t size;
	}*modules;
	int16_t moduleCount = 0;
	MemoryAllocator* GetProperAlloc(addr_t _Addr,size_t _Size);
protected:
	MemoryAllocator* kernelInitAllocator = nullptr;
	MemoryAllocator* kernelPageAllocator = nullptr;
	MemoryAllocator* userObjectAllocator = nullptr;
	bool InitInitAllocator();
	bool ArrangeMemoryLayout();
	bool MoveModulesToSafe();
	void Reserve(addr_t _Addr,size_t _Size);
	void Dereserve(addr_t _Addr,size_t _Size);
public:
	//new运算符每次执行都会调用这个函数来获取合适的Allocator
	MemoryAllocator* OperatorNewCallback(size_t _Size);
	//delete的
	MemoryAllocator* OperatorDeleteCallback(void* _Ptr);
};

extern MemoryManager globalMemoryManager;