//
// Created by ssj on 15-8-13.
//

#pragma once

#include <thread/WaitableObj.h>
#include <ramdisk/RamDiskItem.h>
#include"Global.h"

class Buffer {
private:
    friend class BufferManager;
    int32_t b_blksize;
    int32_t b_bufsize;
    RamDiskItem* b_item = nullptr;
    SpinLock spinlock;

    Buffer* b_next = nullptr,*b_prev = nullptr;
    Buffer* b_next_free = nullptr,*b_prev_free = nullptr;
public:
    void* b_data;
    dev_t b_dev = 0;
    uint32_t b_blocknr = 0;
    bool b_dirt = 0;
    int32_t b_count = 0;
    bool b_lock = 0;
    bool b_uptodate = 0;
    WaitableObj wait;

    void LockBuffer();
    void UnlockBuffer();
    bool IsLocked();
    void WaitOn();
};

