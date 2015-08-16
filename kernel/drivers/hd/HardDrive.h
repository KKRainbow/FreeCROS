//
// Created by ssj on 15-8-15.
//
#pragma once

#include "Global.h"

class RamDiskItemKernel;
class File;
class Buffer;
class Request;
int HdOpen(RamDiskItemKernel* _Item);
int HdRead(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size);
int HdWrite(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size);
Buffer* HdBlockRead(uint32_t _Blocknr, Request* _Request, int _Devnum);
void InitHd();
