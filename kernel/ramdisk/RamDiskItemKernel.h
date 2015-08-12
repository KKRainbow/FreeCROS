//
// Created by ssj on 15-8-12.
//

#pragma once
#include"Global.h"
#include"RamDiskItem.h"

class RamDiskItemKernel;
typedef int (*OpenFunc_t)(RamDiskItemKernel*);
typedef int (*WriteFunc_t)(RamDiskItemKernel*,int8_t*, size_t );
typedef int (*ReadFunc_t)(RamDiskItemKernel*,int8_t*, size_t );
typedef int (*SeekFunc_t)(RamDiskItemKernel*,off_t , int);
class RamDiskItemKernel : public RamDiskItem{
private:
    OpenFunc_t kopen;
    WriteFunc_t kwrite;
    ReadFunc_t kread;
    SeekFunc_t kseek;
public:
    int GetDevnum() const {
        return devnum;
    }

private:
    int devnum;
public:
    RamDiskItemKernel(int32_t _Id,Type _Type,lr::sstl::AString _Name,int _Devnum,
                       OpenFunc_t,ReadFunc_t,WriteFunc_t,SeekFunc_t);
    virtual int Open();
    virtual int Read(int8_t *_Buffer, size_t _Size);
    virtual int Write(int8_t *_Buffer, size_t _Size);
    virtual int Seek(off_t _Offset, int _Whence);
};


