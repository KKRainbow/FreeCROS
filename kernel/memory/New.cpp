void* operator new(unsigned int size)
{
	memSpinLock.Lock();
	if(newInit == false)
	{
		newInit = true;
		MemoryAllocator* tmp = &globalBlockAlloc;
		ObjAlloc = reinterpret_cast<MemoryAllocator*>(tmp->Allocate(PAGE_SIZE,PAGE_SIZE));
		new(ObjAlloc)MemoryObjectAllocator();
		tmp->AddChild(ObjAlloc);
	}
	void *res =  ObjAlloc->Allocate(size,1);
	memset(res,0,size);
	memSpinLock.Unlock();
	return res;
}
void* operator new(unsigned int size,void* p)
{
	size = 0;
	return p;
}
void* operator new[](unsigned int size)
{
	return operator new(size);
}
void* operator new[](unsigned int size,void* p)
{
	return operator new(size,p);
}
void operator delete(void* p) throw()
{
	memSpinLock.Lock();
	ObjAlloc->Deallocate(p);
	memSpinLock.Unlock();
}
