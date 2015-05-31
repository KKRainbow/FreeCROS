#pragma once

static inline bool Checksum(void* from,int size)
{
	unsigned char* f = (unsigned char*)from;
	unsigned int res = 0;
	for(int i = 0;i<size;i++)
	{
		res += f[i];
	}
	return (res&0xff) == 0;
}
