#include"MemoryManager.h"

void *operator new(unsigned int _Size) {
    return MemoryManager::Instance()->KernelObjectAllocate(_Size);
}

void *operator new(unsigned int _Size, void *_Ptr) {
    _Size = 0;
    return _Ptr;
}

void *operator new[](unsigned int _Size) {
    return operator new(_Size);
}

void *operator new[](unsigned int _Size, void *_Ptr) {
    return operator new(_Size, _Ptr);
}

void operator delete(void *_Ptr) throw() {
    auto res = MemoryManager::Instance()->AutoDeallocate(_Ptr);
    Assert(res);
}
