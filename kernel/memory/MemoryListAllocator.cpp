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

#include "MemoryListAllocator.h"
#include"Log.h"

MemoryListAllocator::MemoryListAllocator(addr_t _Start,size_t _Size,MemoryZoneType _Type)
:start(_Start),size(_Size)
{
	this->allocType = _Type;
}

MemoryListAllocator::~MemoryListAllocator()
{

}

bool MemoryListAllocator::Deinitialize()
{
	//Do Nothing
	//没什么事可以做似乎
	return true;
}

bool MemoryListAllocator::Initialize()
{
	if(this->size < sizeof(ListHead))
	{
		//剩余空间连头都放不下
		return false;
	}
	auto head = (ListHead*)this->start;
	
	*head = ListHead();
	head->size = this->size - sizeof(ListHead);
	head->type = ListHead::FREE;
	head->next = head->prev = head;
	
	auto tmp = &guard;
	ListInsertAfter(head,tmp);
	return true;
}

size_t MemoryListAllocator::BlockSize()
{
	//1字节最小
	return 1;
}

void MemoryListAllocator::Dereserve(void* _From, size_t _Size)
{
	//先把Reserve的块加入一个头,类型为Free,并把它加入链表,然后调用Merge即可
	auto head = (ListHead*)_From;
	*head = ListHead();
	head->type = ListHead::FREE;
	head->size = _Size - sizeof(ListHead);
	
	auto leftHead = this->GetHeadFromAddress((addr_t)_From - 1,0);
	auto rightHead = leftHead->next;
	this->CheckMagic(leftHead);
	this->CheckMagic(rightHead);
	
	this->ListInsertAfter(leftHead,head);
	this->MergeFreeHead(head);
}

void MemoryListAllocator::Reserve(void* _From, size_t _Size)
{
	if(_Size == 0)return;
	auto head = this->GetHeadFromAddress((addr_t)_From,_Size);
	if(head == nullptr)
	{
		return;
	}
	//判断头部是否在要预留的空间内
	Assert((addr_t)_From >= (addr_t)head + sizeof(ListHead));
	//看是否已经被使用
	Assert(head->type == ListHead::FREE);
	
	//预留空间,左侧空间设置一个头,右侧空间设置一个头,直接把这部分空间架空
	//右侧的头
	auto rightHead = (ListHead*)((addr_t)_From + _Size);
	*rightHead = ListHead();
	rightHead->type = ListHead::FREE;
	rightHead->size = (addr_t)head + sizeof(ListHead) + head->size
		- (addr_t)rightHead - sizeof(ListHead);
	//左部的头重新利用已经存在的即可
	head->size = (addr_t)_From - ((addr_t)head + sizeof(ListHead));
	
	this->ListInsertAfter(head,rightHead);
}

void MemoryListAllocator::Deallocate(void* _Ptr)
{
	auto listHead = this->GetHeadFromAddress((addr_t)_Ptr);
	this->CheckMagic(listHead);
	
	Assert(listHead->type == ListHead::OCCUPIED);
	
	listHead->type = ListHead::FREE;
	this->MergeFreeHead(listHead);
}

void* MemoryListAllocator::Allocate(size_t _Size, int _Align)
{
	if(_Size == 0)return nullptr;
	//暂时采用First fit
	//防止第一次执行就跳出
	for(auto head = (ListHead*)this->start;head!=&guard;head = head->next)
	{
		this->CheckMagic(head);
		if(head->type != ListHead::FREE)continue;
		addr_t availStart = (addr_t)head + sizeof(ListHead);
		addr_t availEnd = availStart + head->size;
		
		//把availStart按_Align向上对齐
		addr_t realStart = ALIGN_UP(availStart,_Align);
		//看现在是否还满足要求,别忘了在末尾还要追加一个Head呢
		if(availEnd - realStart < _Size + sizeof(ListHead))continue;
		
		//就采用这个了!
		auto newhead = (ListHead*)(realStart + _Size);
		*newhead = ListHead();
		newhead->type = ListHead::FREE;
		newhead->size = availEnd - (realStart + _Size + sizeof(ListHead));
		this->ListInsertAfter(head,newhead);
		
		//设置原来的head
		head->type = ListHead::OCCUPIED;
		head->size = realStart + _Size - availStart;
		return (void*)realStart;
	}
	return nullptr;
}

void MemoryListAllocator::MergeFreeHead(ListHead*& _Obj)
{
	while(1)
	{
		this->CheckMagic(_Obj);
		this->CheckMagic(_Obj->next);
		this->CheckMagic(_Obj->prev);
		//next的地址必须在Obj地址的右边,否则合并出来形状不对= =
		if(_Obj->next != _Obj && _Obj->next->type == ListHead::FREE && 
			(addr_t)_Obj->next > (addr_t)_Obj
		)
		{
			auto sizeOfNext = _Obj->next->size;
			this->ListRemove(_Obj->next);
			_Obj->size += sizeof(ListHead) + sizeOfNext;
			continue;
		}
		else if(_Obj->prev != _Obj && _Obj->prev->type == ListHead::FREE &&
			(addr_t)_Obj->prev < (addr_t)_Obj
		)
		{
			//转换为next为Free的情况.
			_Obj = _Obj->prev;
			continue;
		}
		else
		{
			//两边都不是Free,合并完成.
			break;
		}
	}
}
MemoryListAllocator::ListHead* MemoryListAllocator::GetHeadFromAddress(addr_t _Addr,size_t _Size)
{
	auto head = (ListHead*)nullptr;
	
	for(auto head = (ListHead*)this->start;head!=&guard;head = head->next)
	{
		this->CheckMagic(head);
		if((addr_t)_Addr >= (addr_t)head && 
			(addr_t)_Addr + _Size <= (addr_t)head + head->size + sizeof(ListHead))
		{
			return head;
		}
	}
	//正常情况下绝对不会运行到这
	LOG("Unknow addr deleted! : 0x%x",_Addr);
	Assert(false);
	return nullptr;
}

size_t MemoryListAllocator::FreeSize()
{
	
	size_t freeSize = 0;;
	for(auto head = (ListHead*)this->start;head!=&guard;head = head->next)
	{
		this->CheckMagic(head);
		if(head->type == ListHead::FREE)
		{
			freeSize += head->size;
		}
	}
	return freeSize;
}

void MemoryListAllocator::PrintList()
{
	for(auto head = (ListHead*)this->start;head!=&guard;head = head->next)
	{
		this->CheckMagic(head);
		if(head->type == ListHead::FREE)
		{
			
		}
	}
}