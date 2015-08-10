//
// Created by ssj on 15-8-9.
//

#include "./New.h"
#include "../libkernel/memory/MemoryListAllocator.h"
#include "../libkernel/memory/MemoryListAllocator.cpp"

static MemoryListAllocator globalAlloc(0,0,MemoryZoneType::ORDINARY_PAGE_ALLOC);
static bool flag = false;
void init()
{
    if (!flag){
        new (&globalAlloc)MemoryListAllocator(0x30000000,1<<30,(MemoryZoneType::ORDINARY_PAGE_ALLOC));
        globalAlloc.Initialize();
    }
}
void* operator new(unsigned int _Size)
{
    init();
    return globalAlloc.Allocate(_Size, 0);
}
void* operator new(unsigned int _Size,void* _Ptr)
{
    _Size = 0;
    return _Ptr;
}
void* operator new[](unsigned int _Size)
{
    return operator new(_Size);
}
void* operator new[](unsigned int _Size,void* _Ptr)
{
    return operator new(_Size,_Ptr);
}
void operator delete(void* _Ptr) throw()
{
    init();
    globalAlloc.Deallocate(_Ptr);
}
