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

#include"MemoryAllocator.h"
#include"stl/stuple.h"

class MemoryBuddyAllocator :  public MemoryAllocator
{
private:
	char** avail;
	static const char INAVAILABLE = 0;
	static const char AVAILABLE = 1;
	static const char OCCUPIED = 2;
	
	MemoryAllocator* alloc;
	addr_t start;
	addr_t size;
	addr_t unit;
	size_t availSize;
	
	lr::sstl::Tuple<int,int> GetCoordsOfAddr(addr_t _Addr);
	lr::sstl::Tuple<addr_t,size_t> GetPropertyCorrespondToAvailElem(int _Block,int _Num);
	lr::sstl::Tuple<addr_t,int,int>
		SplitLargeSpace(size_t _Size,int _Align,int _Block,int _Num,addr_t _Addr = 0);
	void MergeBuddy(int _Block,int _Num);
	//如果可以容纳,就返回对齐后的地址,否则返回0
	addr_t CheckFit(size_t _Size,int _Align,addr_t _BAddr,size_t _BSize);
	lr::sstl::Tuple<int,int> GetFirstFit(size_t _Size,int _Align,bool _IsFree = false);
public:
	MemoryBuddyAllocator(addr_t _Start,size_t _Size,size_t _Unit,MemoryAllocator* _Alloc,
					   MemoryZoneType _Type);
	~MemoryBuddyAllocator();
	virtual bool Deinitialize();
	virtual bool Initialize();
	virtual size_t BlockSize();
	virtual void Dereserve(void* _Free, size_t _Size);
	virtual void Reserve(void* _From, size_t _Size);
	virtual void Deallocate(void* _Ptr);
	virtual void* Allocate(size_t _Size, int _Align);
	
	friend void PrintList();
};

