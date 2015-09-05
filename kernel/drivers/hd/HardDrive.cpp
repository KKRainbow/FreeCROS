//
// Created by ssj on 15-8-15.
//

#include "Global.h"
#include "stl/slinkedlist.h"
#include "ramdisk/RamDiskItemKernel.h"
#include <ramdisk/RamDisk.h>
#include <drivers/buffer/BufferManager.h>
#include <driver/buffer/Request.h>
#include <thread/ThreadManager.h>
#include <Clock.h>
#include <cpu/CPUManager.h>
#include "HardDrive.h"

#define BLOCK_SIZE 1024
using namespace lr::sstl;
extern Request* request;

int HdOpen(RamDiskItemKernel* _Item)
{
    return 1;
}

Buffer* HdBlockRead(uint32_t _Blocknr, Request* _Request, int _Devnum)
{
    Buffer* bh;

    if (!(bh=BufferManager::Instance()->GetBuffer(_Devnum,_Blocknr,BLOCK_SIZE)))
    {
        LOG("bread: getblk returned NULL\n");
        Assert(false);
        return nullptr;
    }
    bh->LockBuffer();
    if (!BufferManager::Instance()->IsReadNecessary(bh))
        return bh;
    _Request->MakeRequest(Req::READ, bh, -1);
    bh->LockBuffer();
    if (bh->b_uptodate)
        return bh;
    BufferManager::Instance()->BufferRelease(bh);
    return nullptr;
}
enum ComType{READ,WRITE};
static int HdReadWrite(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size,ComType command)
{
    if(_Size <= 0)return 0;
    //计算所属块号
    int start = _Fptr->f_pos;
    int end = _Fptr->f_pos + _Size;
    int block_start = start / BLOCK_SIZE;
    int block_end = (end + BLOCK_SIZE) / BLOCK_SIZE;
    int left = _Size;
    Buffer* bh;
    uint32_t eflags;
    //读第一个
    if (!(bh = HdBlockRead(block_start, request, _Item->GetID())))
    {
        return 0;
    }
    else
    {
        int offset = start % BLOCK_SIZE;
        int size = BLOCK_SIZE - offset;
        if(size > left)size = left;
        if (command == READ)
        {
            memcpy(_Buffer, bh->b_data + offset, size);
        }
        else
        {
            bh->b_dirt = true;
            memcpy(bh->b_data + offset,_Buffer, size);
        }
        _Buffer += size;
        left -= size;
        bh->UnlockBuffer();
        BufferManager::Instance()->BufferRelease(bh);
    }

    for (int i = block_start + 1 ; i < block_end; i++)
    {
        if (!(bh = HdBlockRead(i, request, _Item->GetDevnum())))
        {
            goto end;
        }
        else
        {
            int size = left > BLOCK_SIZE ? BLOCK_SIZE : left;
            if (command == READ)
            {
                memcpy(_Buffer, bh->b_data, size);
            }
            else
            {
                bh->b_dirt = true;
                memcpy(bh->b_data,_Buffer, size);
            }
            _Buffer += size;
            left -= size;
            bh->UnlockBuffer();
            BufferManager::Instance()->BufferRelease(bh);
        }
    }
    end:
    return _Size - left;

}

int HdRead(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size)
{
    return HdReadWrite(_Fptr, _Item, _Buffer, _Size, READ);
}

int HdWrite(File* _Fptr, RamDiskItemKernel* _Item,int8_t* _Buffer,size_t _Size)
{
    return HdReadWrite(_Fptr, _Item, _Buffer, _Size, WRITE);
}

void hd_init();

bool hd_callback(Buffer* _Bh)
{
    if (RamDisk::Instance()->GetItemByID(_Bh->b_dev)->GetName().Sub(0,2) != "hd")
        return false;
    request->MakeRequest(Req::WRITE, _Bh, -1);
    return true;
}
void hd_sync()
{
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
    uint32_t eflags;
    while(1)
    {
        int succ,total;
        int i = BufferManager::Instance()->SyncBuffers(hd_callback,succ,total);
        if(i)
        {
            LOG("Sync %d buffers\n",i);
        }
        if(total)
        {
//            LOG("Total %d buffers\n",total);
        }
        LOG("Total %d buffers,at: %d\n",total, CPUManager::Instance()->GetClockCounter());
    }
#pragma clang diagnostic pop
}

void InitHd()
{
    hd_init();
    Thread *t = ThreadManager::Instance()->CreateThread(ThreadType::KERNEL);
    if(!t)return;

    t->SetEntry((addr_t)hd_sync);
    t->State()->ToReady(t);
}
