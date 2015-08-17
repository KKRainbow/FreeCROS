#pragma once

#include"Global.h"


class SpinLock {
private:
    char tmplock;
    int depth;
    int currCPU;
    static bool basicMode;
    uint32_t eflag;
    char lock;
public:
    void Lock();

    void Unlock();

    bool Try();

    static void SetBasicMode(bool _BasicMode) {
        SpinLock::basicMode = _BasicMode;
    }

    SpinLock();
};

