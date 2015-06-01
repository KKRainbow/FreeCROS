void* operator new(unsigned int size)
{
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
}
