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
#ifndef __SERVER
#define __DEBUG
#endif

#include"Log.h"
#ifdef __DEBUG
//Die
static void AssertFunc(bool cond)
{
	if(!cond)
	{
		for(;;);
	}
}
#define Assert(cond) do {			\
	if(!(cond)){	\
	LOG(__FILE__);		\
	LOG(":");		\
	LOG("%d",__LINE__);		\
	LOG("\n");}		\
	AssertFunc(cond);\
}while(0)

#else

#define Assert(cond) do{if(cond)do{}while(0);}while(0)

#endif
