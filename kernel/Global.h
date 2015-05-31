#pragma once

#include"Macros.h"
#include"Type.h"


extern unsigned long kernelStart;
extern unsigned long code;
extern unsigned long data;
extern unsigned long bss;
extern unsigned long kernelEnd;

//Die
static void Assert(bool cond)
{
	if(!cond)
	{
		for(;;);
	}
}