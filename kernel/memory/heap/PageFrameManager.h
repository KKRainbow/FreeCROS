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
#include <memory/MemoryAllocator.h>
#include"thread/ThreadManager.h"

class PageFrameManager
{
	SINGLETON_H(PageFrameManager)
	struct PageFrame
	{
		uint8_t present; //是否已经通过Alloc获取？
		Thread* currThread = nullptr; //如果没有被使用则为nullptr
		uint8_t writable = 0; //是否可写
		addr_t addr;
	};
private:
	MemoryAllocator* userPageAlloc = nullptr;
	lr::Ptr<PageFrame> pageframes;
	inline uint32_t PFN(addr_t _Addr)
	{
		Assert((_Addr & (PAGE_SIZE - 1)) == 0);
		Assert(_Addr >= userPageAlloc->StartAddr());
		Assert(_Addr < userPageAlloc->StartAddr() + userPageAlloc->Size());
		return (_Addr - userPageAlloc->StartAddr()) >> PAGE_SHIFT;
	}
	PageFrame* GetPageFrame(uint32_t _PFN);
public:
	/**
	 * @return address of page frame
	 */
	PageFrame* GetNewPageFrame();
	void PutPageFrame(PageFrame* _PF);
};
