//
// Created by ssj on 15-8-18.
//

#include "stdio.h"
#include "SystemCalls.h"

FILE fp_table[FP_TABLE_SIZE];
FILE* stdin;
FILE* stderr;
FILE* stdout;

extern "C" FILE * fopen(const char *filename,
                        const char *mode)
{
    FILE* fp = nullptr;
    for (auto& ft : fp_table)
    {
        if(ft.fd < 0)
        {
            fp = &ft;
            break;
        }
    }
    if(fp == nullptr)return nullptr;
    int fd = SysCallOpen::Invoke((uint32_t)filename, (uint32_t)mode);
    if(fd < 0)return nullptr;

    fp->fd = fd;
    fp->lock = 0;
    return fp;
}

extern "C" size_t fread(void *ptr, size_t size,
                        size_t nitems, FILE *stream)
{
    size_t res = SysCallRead::Invoke((uint32_t)stream->fd,(uint32_t)ptr,(uint32_t)(size * nitems));
    return res - (res % size);
}

extern "C" size_t fwrite(const void *ptr, size_t size, size_t nitems,FILE *stream)
{
    size_t res = SysCallWrite::Invoke((uint32_t)stream->fd,(uint32_t)ptr,(uint32_t)(size * nitems));
    if (res == 0)
    {
        printf("\nwhat the fuck?!!!\n");
    }
    return res - (res % size);
}

extern "C" int fseek(FILE *stream, long offset, int whence)
{
    int res = SysCallSeek::Invoke((uint32_t)stream->fd,(uint32_t)offset,(uint32_t)whence);
    return res;
}

extern "C" long ftell(FILE* stream)
{
    int res = SysCallSeek::Invoke((uint32_t)stream->fd,(uint32_t)0,(uint32_t)SEEK_CUR);
    return res;
}

extern "C" int fclose(FILE *stream)
{
    return SysCallClose::Invoke((uint32_t)stream->fd);
}

extern "C" void flock(FILE* stream)
{
    SpinLock& lock = stream->slock;
    lock.Lock();
    while(stream->lock)
    {
        lock.Unlock();
        SysCallGiveUp::Invoke();
        lock.Lock();
    }
    stream->lock = 1;
    lock.Unlock();
}

extern "C" void funlock(FILE* stream)
{
    SpinLock& lock = stream->slock;
    lock.Lock();
    stream->lock = 0;
    lock.Unlock();
}
