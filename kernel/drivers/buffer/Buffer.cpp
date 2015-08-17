//
// Created by ssj on 15-8-13.
//

#include <Interrupt.h>
#include "driver/buffer/Buffer.h"

void Buffer::LockBuffer() {
    spinlock.Lock();
    while(this->b_lock)
    {
        spinlock.Unlock();
        this->wait.Wait();
        spinlock.Lock();
    }
    this->b_lock = true;
    spinlock.Unlock();
}

void Buffer::UnlockBuffer() {
    this->b_lock = false;
    this->wait.Wake();
}

bool Buffer::IsLocked() {
    return this->b_lock;
}

void Buffer::WaitOn() {
    while(this->b_lock)
    {
        this->wait.Wait();
    }
}
