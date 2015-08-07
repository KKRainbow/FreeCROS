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

#include "PageFrameManager.h"
#include <memory/MemoryManager.h>

SINGLETON_CPP(PageFrameManager)
{
	this->userPageAlloc = MemoryManager::Instance()->GetUserPageAllocator();
	
	//一共有多少页帧？即最大页帧号
	uint32_t pfCount = PFN(
		this->userPageAlloc->StartAddr() +
			this->userPageAlloc->Size() - PAGE_SIZE
			    );
	//必须大于0 
	Assert(pfCount);
	
	this->pageframes = new PageFrame[pfCount];
	Assert(this->pageframes);
}

PageFrameManager::PageFrame* PageFrameManager::GetNewPageFrame()
{
	addr_t pageAddr = (addr_t)this->userPageAlloc->Allocate(PAGE_SIZE);
	
	if(pageAddr == 0)return nullptr;
	else
	{
		return &this->pageframes[PFN(pageAddr)];
	}
}

void PageFrameManager::PutPageFrame(PageFrame* _PF)
{
	auto pf = GetPageFrame(PFN(_PF->addr));
	Assert(pf->present == 1);
	this->userPageAlloc->Deallocate((void*)pf->addr);
	
	*pf = PageFrame();
}

PageFrameManager::PageFrame* PageFrameManager::GetPageFrame(uint32_t _PFN)
{
	return &this->pageframes[_PFN];
}

