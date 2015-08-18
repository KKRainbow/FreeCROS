//
// Created by ssj on 15-8-18.
//

#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "SystemCalls.h"
#include "fileops.h"

extern "C" int fprintf(FILE *stream, const char *format, ...)
{
    if(!stream)return 0;
    int ret;
    char buffer[1024];
    va_list list;

    memset(buffer,0,400);
    va_start(list,format);
    ret = vsnprintf(buffer, 500, format, list);
    va_end(list);


    return SysCallWrite::Invoke(stream->fd, (uint32_t) buffer, (uint32_t)strlen(buffer));
}
