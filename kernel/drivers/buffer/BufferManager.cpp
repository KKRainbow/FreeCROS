//
// Created by ssj on 15-8-12.
//

#include <memory/MemoryManager.h>
#include "BufferManager.h"

SINGLETON_CPP(BufferManager)
{
    memset(this->hash_table,0,sizeof(this->hash_table[0]) * NR_HASH);
}

Buffer *BufferManager::FindBufferFromHashTable(dev_t _Dev, uint32_t _Block) {
    for (Buffer* tmp = hash(_Dev,_Block) ; tmp != nullptr ; tmp = tmp->b_next)
        if (tmp->b_dev==_Dev && tmp->b_blocknr==_Block)
            return tmp;
    return nullptr;
}

void BufferManager::RemoveFromQueues(Buffer *_Buf){
    /* remove from hash-queue */
    if (_Buf->b_next)
        _Buf->b_next->b_prev = _Buf->b_prev;
    if (_Buf->b_prev)
        _Buf->b_prev->b_next = _Buf->b_next;
    if (hash(_Buf->b_dev,_Buf->b_blocknr) == _Buf)
        hash(_Buf->b_dev,_Buf->b_blocknr) = _Buf->b_next;
/* remove from free list */
    if (!(_Buf->b_prev_free) || !(_Buf->b_next_free))
    {
        LOG("Free block list corrupted");
        for(;;);
    }
    _Buf->b_prev_free->b_next_free = _Buf->b_next_free;
    _Buf->b_next_free->b_prev_free = _Buf->b_prev_free;
    if (free_list == _Buf)
        free_list = _Buf->b_next_free;
}

void BufferManager::InsertIntoQueues(Buffer *_Buf) {
/* put at end of free list */
    _Buf->b_next_free = free_list;
    _Buf->b_prev_free = free_list->b_prev_free;
    free_list->b_prev_free->b_next_free = _Buf;
    free_list->b_prev_free = _Buf;
/* put the buffer in new hash-queue if it has a device */
    _Buf->b_prev = nullptr;
    _Buf->b_next = nullptr;
    if (!_Buf->b_dev)
        return;
    _Buf->b_next = hash(_Buf->b_dev,_Buf->b_blocknr);
    hash(_Buf->b_dev,_Buf->b_blocknr) = _Buf;
    _Buf->b_next->b_prev = _Buf;
}

Buffer *BufferManager::GetCachedBuffer(dev_t _Dev, uint32_t _Block) {
    Buffer * bh;

    for (;;) {
        if (!(bh=FindBufferFromHashTable(_Dev,_Block)))
            return nullptr;
        bh->b_count++;
        bh->WaitOn();
        if (bh->b_dev == _Dev && bh->b_blocknr == _Block)
            return bh;
        bh->b_count--;
    }
}

Buffer *BufferManager::GetBuffer(dev_t _Dev, uint32_t _Block, uint32_t _BlockSize) {

    Buffer* bh;
    bool oom_flag = false;

    repeat:
    if ((bh = GetCachedBuffer(_Dev,_Block)) != nullptr)
        return bh;
    if (curr_buf_count + 1 < max_buf_count && curr_buf_size + _BlockSize < max_buf_size && !oom_flag)
    {
        bh = new Buffer();
        if(!bh)
        {
            oom_flag = true;
            goto repeat;
        }
    }
    else
    {
        Buffer* tmp = free_list;
        do {
            if (tmp->b_count)
                continue;
            if (!bh || Badness(tmp)<Badness(bh)) {
                bh = tmp;
                if (!Badness(tmp))
                    break;
            }
/* and repeat until we find something good */
        } while ((tmp = tmp->b_next_free) != free_list);
        if (!bh) {
            this->wait.Sleep();
            goto repeat;
        }
        bh->WaitOn();
        if (bh->b_count)
            goto repeat;
        while (bh->b_dirt) {
            SyncDev(bh->b_dev);
            bh->WaitOn();
            if (bh->b_count)
                goto repeat;
        }
/* NOTE!! While we slept waiting for this block, somebody else might */
/* already have added "this" block to the cache. check it */
        if (GetCachedBuffer(_Dev,_Block))
            goto repeat;

    }
/* OK, FINALLY we know that this buffer is the only one of it's kind, */
/* and that it's unused (b_count=0), unlocked (b_lock=0), and clean */
    bh->b_count=1;
    bh->b_dirt=false;
    bh->b_uptodate=false;
    RemoveFromQueues(bh);
    bh->b_dev=_Dev;
    bh->b_blocknr=_Block;
    bh->b_blksize = _BlockSize;
    InsertIntoQueues(bh);
    return bh;
}

void BufferManager::SyncDev(dev_t _Dev) {

}

void BufferManager::ReadWriteBlock(int _Op,Buffer *_Buf) {
    Assert(_Buf);
    if(_Buf->b_blksize > _Buf->b_bufsize)
    {
        Assert(!_Buf->b_uptodate);
        if(_Buf->b_data)
        {
            Assert(_Buf->b_bufsize > 0);
            delete _Buf->b_data;
        }
        _Buf->b_data = new char[_Buf->b_blksize];
        this->curr_buf_size += _Buf->b_blksize - _Buf->b_bufsize;
        _Buf->b_bufsize = _Buf->b_blksize;
    }
    switch (_Op)
    {
        case READ:
            break;
        case WRITE:
            break;
        default:
            LOG("Unknown param,Error!!!!");
            Assert(false);
            return;
    }
}

Buffer *BufferManager::BlockRead(dev_t _Dev, uint32_t _Block, uint32_t _BlockSize) {
    Buffer* bh;

    if (!(bh=GetBuffer(_Dev,_Block,_BlockSize)))
    {
        LOG("bread: getblk returned NULL\n");
        Assert(false);
        return nullptr;
    }
    if (bh->b_uptodate)
        return bh;
    ReadWriteBlock(READ,bh);
    bh->WaitOn();
    if (bh->b_uptodate)
        return bh;
    BlockRelease(bh);
    return nullptr;
}

void BufferManager::BlockRelease(Buffer *_Buf) {
    if(!_Buf)return;
    _Buf->WaitOn();
    Assert(_Buf->b_count-- > 0);
    _Buf->wait.Wake();
}
