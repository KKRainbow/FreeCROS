//
// Created by ssj on 15-8-15.
//

#include "Global.h"
#include "ramdisk/RamDiskItemKernel.h"
#include <ramdisk/RamDisk.h>
#include <drivers/buffer/BufferManager.h>
#include <driver/buffer/Request.h>
#include "HardDrive.h"

#define BLOCK_SIZE 1024
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
    bh->b_count++;
    if (!BufferManager::Instance()->IsReadNecessary(bh))
        return bh;
    bh->LockBuffer();
    _Request->MakeRequest(Req::READ, bh, -1);
    bh->WaitOn();
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
            bh->LockBuffer();
            memcpy(bh->b_data + offset,_Buffer, size);
            bh->b_dirt = 1;
            bh->UnlockBuffer();
        }
        _Buffer += size;
        left -= size;
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
                bh->LockBuffer();
                memcpy(bh->b_data,_Buffer, size);
                bh->b_dirt = 1;
                bh->UnlockBuffer();
            }
            _Buffer += size;
            left -= size;
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
void InitHd()
{
    hd_init();
}
