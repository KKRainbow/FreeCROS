//
// Created by ssj on 15-8-12.
//

#pragma once
#include"Global.h"
#include"RamDiskItem.h"

class RamDiskItemKernel;
typedef int (*OpenFunc_t)(RamDiskItemKernel*);
typedef int (*WriteFunc_t)(File* _Fptr, RamDiskItemKernel*,int8_t*, size_t );
typedef int (*ReadFunc_t)(File* _Fptr, RamDiskItemKernel*,int8_t*, size_t );
typedef int (*SeekFunc_t)(File* _Fptr, RamDiskItemKernel*,off_t , int);
class RamDiskItemKernel : public RamDiskItem{
private:
    OpenFunc_t kopen;
    WriteFunc_t kwrite;
    ReadFunc_t kread;
    SeekFunc_t kseek;
    int blockSize;
    int devnum;
public:
    RamDiskItemKernel(int32_t _Id,Type _Type,lr::sstl::AString _Name,int _Devnum,
                       OpenFunc_t,ReadFunc_t,WriteFunc_t,SeekFunc_t);
    virtual int Open();
    virtual pid_t Read(File *_Fptr, int8_t *_Buffer, size_t _Size);
    virtual pid_t Write(File *_Fptr, int8_t *_Buffer, size_t _Size);
    virtual pid_t Seek(File *_Fptr, off_t _Offset, int _Whence);
    int getBlockSize() const
    {
        return blockSize;
    }
    void setBlockSize(int blockSize)
    {
        this->blockSize = blockSize;
    }
    int GetDevnum() const
    {
        return devnum;
    }
};


