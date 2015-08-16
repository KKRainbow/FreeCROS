//
// Created by ssj on 15-8-15.
//

#include "Global.h"
#include "ramdisk/RamDiskItemKernel.h"
#include <ramdisk/RamDisk.h>
#include <drivers/buffer/BufferManager.h>
#include <driver/buffer/Request.h>
#include "HardDrive.h"

int HdOpen(RamDiskItemKernel* _Item)
{
    return 1;
}

Buffer* HdBlockRead(uint32_t _Blocknr, Request* _Request, int _Devnum)
{
    Buffer* bh;

    if (!(bh=BufferManager::Instance()->GetBuffer(_Devnum,_Blocknr,1024)))
    {
        LOG("bread: getblk returned NULL\n");
        Assert(false);
        return nullptr;
    }
    if (!BufferManager::Instance()->IsReadNecessary(bh))
        return bh;
    bh->LockBuffer();
    bh->b_count++;
    _Request->MakeRequest(Req::READ, bh, -1);
    bh->WaitOn();
    if (bh->b_uptodate)
        return bh;
    BufferManager::Instance()->BufferRelease(bh);
    return nullptr;
}
int HdRead(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size)
{
    return 1;
}

int HdWrite(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size)
{
    return 1;
}

void hd_init();
void InitHd()
{
    hd_init();

    //注册设备
    RamDiskItemKernel* dev = new RamDiskItemKernel(0,RamDiskItem::Type::KERNELBLOCK,
                                                   "tty",0,HdOpen,HdRead,HdWrite,nullptr);
    int id = RamDisk::Instance()->Instance()->CreateKernelDev(dev);
    auto tmp = RamDisk::Instance()->GetItemByID(id);
    if(!tmp)LOG("???????");
}
