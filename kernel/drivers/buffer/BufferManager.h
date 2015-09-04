//
// Created by ssj on 15-8-12.
//
#pragma once
#include"Global.h"
#include"driver/buffer/Buffer.h"

#define NR_HASH 64
#define MAX_BUF_COUNT 1024
#define MAX_BUF_SIZE (512*1024)

class BufferManager {
    SINGLETON_H(BufferManager)
private:
    Buffer* free_list = nullptr;
    Buffer* hash_table[NR_HASH];
    WaitableObj wait;
    Buffer* start_buffer;
    int curr_buf_count = 0;
    int curr_buf_size = 0;
    int max_buf_count = MAX_BUF_COUNT;
    int max_buf_size = MAX_BUF_SIZE;
    Buffer*& hash(dev_t _Dev,uint32_t _Block)
    {
        return hash_table[(_Dev ^ _Block) % NR_HASH];
    }
    int Badness(Buffer* _Buf)
    {
        return (((int)_Buf->b_dirt)<<1) + (int)_Buf->b_lock;
    }
    void RemoveFromQueues(Buffer* _Buf);
    void InsertIntoQueues(Buffer* _Buf);
    Buffer* FindBufferFromHashTable(dev_t _Dev, uint32_t _Block);
    Buffer* GetCachedBuffer(dev_t _Dev, uint32_t _Block);
    const int READ = 0;
    const int WRITE = 1;
    void PrepareBuffer(Buffer* _Buf);
public:
    Buffer* GetBuffer(dev_t _Dev, uint32_t _Block, uint32_t _BlockSize);
    bool IsReadNecessary(Buffer* _Buf);
    void SyncDev(dev_t _Dev);
    void BufferRelease(Buffer* _Buf);
    int SyncBuffers(void (*_Callback)(Buffer *),int& succ,int& totalDirt);
};
