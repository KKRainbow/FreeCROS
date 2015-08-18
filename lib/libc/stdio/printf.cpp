//
// Created by ssj on 15-8-18.
//
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "SystemCalls.h"

extern "C" int printf(const char *format, ...)
{
    int ret;
    char buffer[1024];
    va_list list;

    memset(buffer,0,sizeof(buffer));
    va_start(list,format);
    ret = vsnprintf(buffer, 500, format, list);
    va_end(list);


    return SysCallWrite::Invoke((uint32_t)stdout->fd, (uint32_t) buffer, (uint32_t)strlen(buffer));
}
