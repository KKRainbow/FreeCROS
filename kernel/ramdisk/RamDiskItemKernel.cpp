//
// Created by ssj on 15-8-12.
//

#include "RamDiskItemKernel.h"
RamDiskItemKernel::RamDiskItemKernel(int32_t _Id,Type _Type,lr::sstl::AString _Name,int _Devnum,
                                     OpenFunc_t _Open,
                                     ReadFunc_t _Read,
                                     WriteFunc_t _Write,
                                     SeekFunc_t _Seek)
:RamDiskItem(nullptr,_Id,_Type,_Name),kopen(_Open),kread(_Read),kwrite(_Write),kseek(_Seek),devnum(_Devnum)
{

}
int RamDiskItemKernel::Open() {
    if(kopen)
        return kopen(this);
    return -1;
}

pid_t RamDiskItemKernel::Read(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    if(kread)
        return kread(_Fptr, this,_Buffer,_Size);
    return -1;
}

pid_t RamDiskItemKernel::Write(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    if(kwrite)
        kwrite(_Fptr, this,_Buffer,_Size);
    return -1;
}

pid_t RamDiskItemKernel::Seek(File *_Fptr, off_t _Offset, int _Whence) {
    if(kseek)
        kseek(_Fptr, this,_Offset,_Whence);
    return -1;
}
