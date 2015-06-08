#pragma once
#include"stdio.h"
#include"SystemCalls.h"
#include"string.h"
static void log(const char* fmt,...)
{
	int ret;
	char buffer[500];
	va_list list;
	
	va_start(list,fmt);
	ret = vsnprintf(buffer, 500, fmt, list);
	va_end(list);
	
	SysCallLog::Invoke((uint32_t)buffer,ret,0,0);
}