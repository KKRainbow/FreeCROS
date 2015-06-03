#pragma once

#include"Macros.h"
#include"Type.h"
#include"Tools.h"


extern unsigned long kernelStart;
extern unsigned long code;
extern unsigned long data;
extern unsigned long bss;
extern unsigned long kernelEnd;
extern unsigned long kernelObjInitZoneStart;
extern unsigned long kernelObjInitZoneEnd;

//Die
static void Assert(bool cond)
{
	if(!cond)
	{
		for(;;);
	}
}
