//
// Created by ssj on 15-8-13.
//

#include <Interrupt.h>
#include "driver/buffer/Buffer.h"

void Buffer::LockBuffer() {
    spinlock.Lock();
    Interrupt::EnterCritical(eflags);
    while(this->b_lock)
    {
        spinlock.Unlock();
        this->wait.Wait();
        spinlock.Lock();
    }
    this->b_lock = true;
    Interrupt::LeaveCritical(eflags);
    spinlock.Unlock();
}

void Buffer::UnlockBuffer() {
    spinlock.Lock();
    Assert(this->b_lock);
    this->b_lock = false;
    this->wait.Wake();
    spinlock.Unlock();
}

bool Buffer::IsLocked() {
    return this->b_lock;
}

void Buffer::WaitOn() {
    spinlock.Lock();
    while(this->b_lock)
    {
        spinlock.Unlock();
        this->wait.Wait();
        spinlock.Lock();
    }
    spinlock.Unlock();
}
