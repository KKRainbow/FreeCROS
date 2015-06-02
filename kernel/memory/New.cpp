#include"MemoryManager.h"
void* operator new(unsigned int _Size)
{
	auto alloc = MemoryManager::Instance()->OperatorNewCallback(_Size);
	return alloc->Allocate(_Size);
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
	auto alloc = MemoryManager::Instance()->OperatorDeleteCallback(_Ptr);
	return alloc->Deallocate(_Ptr);
}
