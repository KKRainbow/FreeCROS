//
// Created by ssj on 15-8-9.
//

#include "./New.h"
#include "stdlib.h"

void *operator new(unsigned int _Size) {
    return malloc((size_t)_Size);
}

void *operator new(unsigned int _Size, void *_Ptr) {
    return _Ptr;
}

void *operator new[](unsigned int _Size) {
    return operator new(_Size);
}

void *operator new[](unsigned int _Size, void *_Ptr) {
    return operator new(_Size, _Ptr);
}

void operator delete(void *_Ptr) throw() {
    free(_Ptr);
}
