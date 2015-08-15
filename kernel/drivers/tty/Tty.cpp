//
// Created by ssj on 15-8-12.
//

#include <ramdisk/RamDisk.h>
#include "Tty.h"
#include"Global.h"
#include"driver/tty/tty.h"
#include"ramdisk/RamDiskItemKernel.h"

int TtyOpen(RamDiskItemKernel* _Item)
{
    return 1;
}

int TtyRead(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size)
{
    return tty_read((unsigned int)_Item->GetDevnum(), _Buffer, _Size);
}

int TtyWrite(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size)
{
  return tty_write((unsigned int)_Item->GetDevnum(), _Buffer, _Size);
}

void InitTty()
{
    tty_init();

    //注册设备
    RamDiskItemKernel* dev = new RamDiskItemKernel(0,RamDiskItem::Type::KERNELCHAR,
    "tty",0,TtyOpen,TtyRead,TtyWrite,nullptr);
    int id = RamDisk::Instance()->Instance()->CreateKernelDev(dev);
    auto tmp = RamDisk::Instance()->GetItemByID(id);
    if(!tmp)LOG("???????");
}