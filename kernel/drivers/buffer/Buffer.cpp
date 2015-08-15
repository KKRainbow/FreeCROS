//
// Created by ssj on 15-8-13.
//

#include <Interrupt.h>
#include "Buffer.h"

void Buffer::LockBuffer() {
    spinlock.Lock();
    Interrupt::Cli();
    while(this->b_lock)
    {
        this->wait.Wait();
    }
    this->b_lock = true;
    Interrupt::Sti();
    spinlock.Unlock();
}

void Buffer::UnlockBuffer() {
    spinlock.Lock();
    Interrupt::Cli();
    this->b_lock = false;
    this->wait.Wait();
    Interrupt::Sti();
    spinlock.Unlock();
}

bool Buffer::IsLocked() {
    return this->b_lock;
}

void Buffer::WaitOn() {
    spinlock.Lock();
    Interrupt::Cli();
    while(this->b_lock)
    {
        this->wait.Wait();
    }
    Interrupt::Sti();
    spinlock.Unlock();
}
