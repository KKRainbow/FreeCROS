#pragma once
void* operator new(unsigned int size);
void* operator new(unsigned int size,void* p);
void* operator new[](unsigned int size);
void* operator new[](unsigned int size,void* p);
void operator delete(void* p) throw();
