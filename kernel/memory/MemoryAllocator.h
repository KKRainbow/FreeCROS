#pragma once
#include"Global.h"
#include"stdlib.h"
enum MemoryZoneType
{
	KERNEL_PAGE_ALLOC,	//分配页大小
	KERNEL_ARBITRARY_ALLOC,	//任意大小
	ORDINARY_PAGE_ALLOC	//内核之外区域的分配
};
class MemoryAllocator
{
protected:
	MemoryZoneType allocType;
public:
	virtual ~MemoryAllocator(){};
	
	virtual void* Allocate(size_t _Size,int _Align) = 0;
	virtual void Deallocate(void* _Ptr) = 0;
	virtual void Reserve(void* _From,size_t _Size) = 0;
	virtual void Dereserve(void* _Free,size_t _Size) = 0;
	virtual size_t BlockSize() = 0; //The minumum mem size that this alloc will ret
	virtual bool Initialize() = 0; //Run after constructed and before using it
	virtual bool Deinitialize() = 0; //Run before destroy it
	MemoryZoneType AllocType(){return this->allocType;}
	virtual addr_t StartAddr() = 0;
	virtual size_t Size() = 0;
	virtual size_t FreeSize() = 0;
};
