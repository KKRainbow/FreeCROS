//
// Created by ssj on 15-9-6.
//
#pragma once
#include "RamDiskItem.h"
class RamDiskItemDir;

class RamDiskItemMounted : public RamDiskItem
{
private:
    RamDiskItemDir* mountPoint = nullptr;
    ino_t internalId = -1;
    RamDiskItemMounted* rootItem;
public:
    RamDiskItemMounted(RamDiskItem* _Mounted, ino_t _Inner, IDType _Id,lr::sstl::AString _Path,
                       RamDiskItemMounted* _Root = nullptr);
    virtual pid_t Open();
    virtual pid_t Read(File *_Fptr, int8_t *_Buffer, size_t _Size);
    virtual pid_t Write(File *_Fptr, int8_t *_Buffer, size_t _Size);
    virtual pid_t Seek(File *_Fptr, off_t _Offset, int _Whence);
    int Mkdir(int mode, bool _Recursive = false);
};
