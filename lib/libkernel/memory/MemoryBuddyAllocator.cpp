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

#include "MemoryBuddyAllocator.h"
#include "string.h"

using namespace lr::sstl;
MemoryBuddyAllocator::MemoryBuddyAllocator(addr_t _Start,size_t _Size,
					   size_t _Unit,MemoryAllocator* _Alloc,
					   MemoryZoneType _Type
  					)
:start(_Start),size(_Size),alloc(_Alloc),unit(_Unit)
{
	this->allocType = _Type;
}

MemoryBuddyAllocator::~MemoryBuddyAllocator()
{

}

bool MemoryBuddyAllocator::Deinitialize()
{
	this->alloc->Deallocate(&(this->avail[0][0]));
	this->alloc->Deallocate(this->avail);
	return true;
}

bool MemoryBuddyAllocator::Initialize()
{
	//计算出一共需要多少空间来存储avail数组
	
	//_Size应该向下关于_Unit对齐
	//最小单元的个数
	size_t sizeOfMinUnit = this->size / this->unit;
	size_t sizeOfAvail = sizeOfMinUnit * 2 * sizeof(**avail);
	
	void* avail = this->alloc->Allocate(sizeOfAvail,0);
	if(avail == nullptr)return false;
	memset(avail,INAVAILABLE,sizeOfAvail);
	
	
	//为avail数组申请空间,该数组不会超过32个元素,所以就申请32个了
	this->avail = (decltype(this->avail))this->alloc->Allocate(32*sizeof(this->avail[0]),0);
	if(this->avail == nullptr)
	{
		this->alloc->Deallocate(avail);
		return false;
	}
	
	
	int i = 0;
	size_t nextSize = sizeOfMinUnit;
	while(nextSize > 0)
	{
		Assert(i < 32);
		this->avail[i++] = (decltype(*(this->avail)))avail;
		
		avail = (void*)((addr_t)avail + nextSize * sizeof(this->avail[0][0]));
		nextSize >>= 1;
		
		this->availSize = i;
	}

	//最大的块当然是可用的
	this->avail[this->availSize - 1][0] = AVAILABLE;
	return true;
}

size_t MemoryBuddyAllocator::BlockSize()
{
	return this->unit;
}

void MemoryBuddyAllocator::Dereserve(void* _Free, size_t _Size)
{
	this->Deallocate(_Free);
}

void MemoryBuddyAllocator::Reserve(void* _From, size_t _Size)
{
	if(_Size == 0)return;
	auto coord = this->GetCoordsOfAddr((addr_t)_From);
	
	auto i = Get<0>(coord);
	auto j = Get<1>(coord);
	
	Assert(i>=0&&j>=0);
	Assert(this->avail[i][j] == AVAILABLE);
	
	auto property = this->GetPropertyCorrespondToAvailElem(i,j);
	
	auto blockAddr = Get<0>(property);
	auto blockSize = Get<1>(property);
	
	Assert(blockSize + blockAddr >= (addr_t)_From + _Size);
	
	auto afterSplit =  this->SplitLargeSpace(_Size,0,i,j,(addr_t)_From);
	auto newi = Get<1>(afterSplit);
	auto newj = Get<2>(afterSplit);
	
	this->avail[newi][newj] = OCCUPIED;
}

void MemoryBuddyAllocator::Deallocate(void* _Ptr)
{
	auto currprop = this->GetCoordsOfAddr((addr_t)_Ptr);
	
	int i = Get<0>(currprop);
	int j = Get<1>(currprop);
	
	this->avail[i][j] = AVAILABLE;
	this->MergeBuddy(i,j);
}

void* MemoryBuddyAllocator::Allocate(size_t _Size, int _Align)
{
	if(_Size == 0)return nullptr;
	auto coord = this->GetFirstFit(_Size,_Align,true);
	auto resi = Get<0>(coord);
	auto resj = Get<1>(coord);
	
	if(resi == -1 || resj == -1)return nullptr;
	auto afterSplit = this->SplitLargeSpace(
		_Size,_Align,resi,resj
	);
	
	auto newi = Get<1>(afterSplit);
	auto newj = Get<2>(afterSplit);
	
	this->avail[newi][newj] = OCCUPIED;
	
	return (void*)Get<0>(afterSplit);
}

lr::sstl::Tuple<int,int> MemoryBuddyAllocator::GetCoordsOfAddr(addr_t _Addr)
{
	Tuple<int,int> bad = MakeTuple(-1,-1);
	
	if(_Addr >= this->start + this->size)return bad;
	
	addr_t currentBlockSize = this->unit;
	for(int i = 0;i < this->availSize;i++)
	{
		//这种方式可以处理有对齐的情况
		addr_t relativeOffset = _Addr - this->start;
		int num = relativeOffset/currentBlockSize;
		if(this->avail[i][num] != INAVAILABLE)
		{
			return MakeTuple(i,num);
		}
		currentBlockSize <<= 1;
	}
	return bad;
}
//获取相对应元素的起始地址和块大小
lr::sstl::Tuple<addr_t,size_t> 
MemoryBuddyAllocator::GetPropertyCorrespondToAvailElem(int _Block,int _Num)
{
	Assert(_Block>=0 && _Block < 32);
	Assert(_Num >= 0);
	
	size_t blockSize = this->unit << _Block;
	size_t sizeOfMinUnit = this->size / this->unit;
	sizeOfMinUnit >>= _Block;
	
	Assert(_Num < sizeOfMinUnit);
	
	return MakeTuple(this->start + _Num * blockSize,blockSize);
}

lr::sstl::Tuple<addr_t,int,int>
	MemoryBuddyAllocator::SplitLargeSpace(size_t _Size,int _Align,int _Block,
					      int _Num,addr_t _Addr)
{
	auto resi = _Block;
	auto resj = _Num;
	auto prop = this->GetPropertyCorrespondToAvailElem(resi,resj);
	addr_t resAddr = 
		this->CheckFit(_Size,_Align,Get<0>(prop),Get<1>(prop));
	Assert(resi < this->availSize && resi >= 0);
	Assert(resj >= 0);
	Assert(resAddr != 0);
	
	while(resi > 0)
	{
		//这个空间是可以容纳的
		//尝试对这个空间进行分割
		int smallBlockNum = resi - 1;
		int smallNum = resj << 1;
		//此时更小块的状态肯定是INAVAILABLE
		Assert(this->avail[smallBlockNum][smallNum] ==
		INAVAILABLE);
		Assert(this->avail[smallBlockNum][smallNum + 1] ==
		INAVAILABLE);
		
		auto leftAvail = false,rightAvai = false;
		auto leftAddr = 0,rightAddr = 0;
		//分割后左边的块符合要求?
		auto leftprop = this->GetPropertyCorrespondToAvailElem
		(smallBlockNum,smallNum);
		leftAddr =  
			this->CheckFit(_Size,_Align,Get<0>(leftprop),Get<1>(leftprop));
		if(leftAddr != 0)
		{
			leftAvail = true;
			//如果addr不为0,还要判断是否包含这个地址范围
			auto bsize = Get<1>(leftprop);
			if(_Addr != 0 && (leftAddr > _Addr || Get<0>(leftprop) + bsize < 
				_Addr + _Size)
			)
			{
				leftAvail = false;
			}
		}
		
		//右边呢?
		if(leftAvail == false)
		{
			auto rightprop = this->GetPropertyCorrespondToAvailElem
			(smallBlockNum,smallNum + 1);
			rightAddr =  
				this->CheckFit(_Size,_Align,Get<0>(rightprop),Get<1>(rightprop));
			if(rightAddr != 0)
			{
				rightAvai = true;
				auto bsize = Get<1>(rightprop);
				if(_Addr != 0 && (rightAddr > _Addr || Get<0>(rightprop) + bsize < 
					_Addr + _Size)
				)
				{
					leftAvail = false;
				}
			}
		}
		
		if(leftAvail || rightAvai)
		{
			this->avail[smallBlockNum][smallNum] = AVAILABLE;
			this->avail[smallBlockNum][smallNum + 1] = AVAILABLE;
			this->avail[resi][resj] = INAVAILABLE;
			resi = smallBlockNum;
			if(leftAvail)
			{
				resj = smallNum;
				resAddr = leftAddr;
			}
			else
			{
				resj = smallNum + 1;
				resAddr = rightAddr;
			}
			continue;
		}
		break;
	}
	return MakeTuple(resAddr,resi,resj);
}
void MemoryBuddyAllocator::MergeBuddy(int _Block,int _Num)
{
	auto curri = _Block;
	auto currj = _Num;
	Assert(curri >= 0 && currj >= 0);
	Assert(this->avail[curri][currj] != INAVAILABLE);
	
	
	while(1)
	{
		if(curri >= this->availSize)return;
		//利用下面这个函数来检验currj是否超出范围.
		//这个函数里面有Assert
		this->GetPropertyCorrespondToAvailElem(curri,currj);
		
		auto largeI = curri + 1;
		auto largeJ = currj >> 1;
		if(largeI >= this->availSize)return;
		
		Assert(this->avail[largeI][largeJ] ==
			INAVAILABLE);
		
		auto leftI = curri;
		auto leftJ = (currj & 1) == 0 ? currj:currj - 1;
		
		auto rightI = curri;
		auto rightJ = currj | 1;
		
		if(this->avail[leftI][leftJ] == AVAILABLE && 
			this->avail[rightI][rightJ] == AVAILABLE)
		{
			this->avail[leftI][leftJ] = INAVAILABLE;
			this->avail[rightI][rightJ] = INAVAILABLE;
			this->avail[largeI][largeJ] = AVAILABLE;
			
			curri = largeI;
			currj = largeJ;
		}
		else //无法继续合并
		{
			return;
		}
	}
}
addr_t MemoryBuddyAllocator::CheckFit(size_t _Size,int _Align,addr_t _BAddr,size_t _BSize)
{
	if(_BSize < _Size)return 0;
	//把_BAddr向上对齐
	addr_t realAddr = ALIGN_UP(_BAddr,_Align);
	
	if(_BAddr + _BSize - realAddr >= _Size)
	{
		return realAddr;
	}
	return 0;
}

lr::sstl::Tuple<int,int> MemoryBuddyAllocator::GetFirstFit(size_t _Size,int _Align,bool _IsFree)
{
	size_t sizeOfMinUnit = this->size / this->unit;
	//找到第一个可以容纳的
	int resi = -1,resj = -1,resAddr = 0;
	for(int i = 0;i<this->availSize;i++)
	{
		for(int j = 0;j<sizeOfMinUnit;j++)
		{
			if((_IsFree && this->avail[i][j] == AVAILABLE)
				|| (!_IsFree && this->avail[i][j] != INAVAILABLE)
			)//该空间可用
			{
				//计算属于该空间的
				auto prop = this->GetPropertyCorrespondToAvailElem(i,j);
				addr_t realAddr = 
					this->CheckFit(_Size,_Align,Get<0>(prop),Get<1>(prop));
				if(realAddr != 0)
				{
					return MakeTuple(i,j);
				}
				//否则继续测试下一个块大小
				break;
			}
		}
		sizeOfMinUnit >>= 1;
	}
	return MakeTuple(-1,-1);
}
size_t MemoryBuddyAllocator::FreeSize()
{
	size_t sizeOfMinUnit = this->size / this->unit;
	//找到第一个可以容纳的
	size_t freeSize = 0;
	for(int i = 0;i<this->availSize;i++)
	{
		for(int j = 0;j<sizeOfMinUnit;j++)
		{
			if(this->avail[i][j] == MemoryBuddyAllocator::AVAILABLE)//该空间可用
			{
				freeSize += 
					this->size/sizeOfMinUnit;
			}
		}
		sizeOfMinUnit >>= 1;
	}
	return freeSize;
}