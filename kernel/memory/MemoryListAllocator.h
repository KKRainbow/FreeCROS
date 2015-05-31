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

class MemoryListAllocator :  MemoryAllocator
{
private:
	struct ListHead
	{
		static const int MAGIC = 0x12345678;
		int magic = MAGIC;
		ListHead* prev = nullptr;
		ListHead* next = nullptr;
		enum{OCCUPIED,FREE,RESERVE}type;
		
		//这里的size是除去Head之后的!!
		size_t size = 0;
	};
	addr_t start;
	size_t size;
	
	void CheckMagic(ListHead* _Head)
	{
		Assert(_Head && _Head->magic == ListHead::MAGIC);
	}
	
	void ListInsertBefore(ListHead*& _Target,ListHead*& _Obj)
	{
		_Obj->next = _Target->next;
		_Obj->prev = _Target;
		
		_Target->next->prev = _Obj;
		
		_Target->next = _Obj;
	}
	void ListInsertAfter(ListHead*& _Target,ListHead*& _Obj)
	{
		_Obj->next = _Target;
		_Obj->prev = _Target->prev;
		
		_Target->prev->next = _Obj;
		
		_Target->prev = _Obj;
	}
	void ListRemove(ListHead*& _Obj)
	{
		auto& prev = _Obj->prev;
		auto& next = _Obj->next;
		
		prev->next = next;
		next->prev = prev;
		
		_Obj->next = _Obj->prev = nullptr;
	}
	void MergeFreeHead(ListHead*& _Obj);
	ListHead* GetHeadFromAddress(addr_t _Addr,size_t _Size = 0);//根据地址获取head指针
public:
	MemoryListAllocator(addr_t _Start,size_t _Size,MemoryZoneType _Type);
	virtual ~MemoryListAllocator()override;
	virtual bool Deinitialize()override;
	virtual bool Initialize()override;
	virtual size_t BlockSize()override;
	virtual void Dereserve(void* _Free, size_t _Size)override;
	virtual void Reserve(void* _From, size_t _Size)override;
	virtual void Deallocate(void* _Ptr)override;
	virtual void* Allocate(size_t _Size, int _Align)override;
};

