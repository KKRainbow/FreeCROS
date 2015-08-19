//
// Created by ssj on 15-8-18.
//


#include "stdlib.h"
#define __SERVER
#include "../libkernel/memory/MemoryListAllocator.h"
#include "../libkernel/memory/MemoryListAllocator.cpp"

static MemoryListAllocator globalAlloc(0, 0, MemoryZoneType::ORDINARY_PAGE_ALLOC);
static bool flag = false;

void init() {
    if ( !flag ) {
        new(&globalAlloc)MemoryListAllocator(0x30000000, 1 << 30, (MemoryZoneType::ORDINARY_PAGE_ALLOC));
        globalAlloc.Initialize();
        flag = true;
    }
}
extern "C" void * malloc(size_t _Size)
{
    init();
    return globalAlloc.Allocate(_Size, 0);
}
extern "C" void free(void *_Ptr)
{
    init();
    globalAlloc.Deallocate(_Ptr);
}
