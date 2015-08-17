#pragma once

#include"Global.h"

enum States {
    RUNNING, READY, INTERRUPTABLE, UNINTERRUPTABLE, ZOMBIE
};

class Thread;

class ThreadState {
public:
    static ThreadState *GetState(States _State);

    virtual bool ToRun(Thread *_Thread) = 0;

    virtual bool ToIOBlocked(Thread *_Thread) = 0;

    virtual bool ToPause(Thread *_Thread) = 0;

    virtual bool ToReady(Thread *_Thread) = 0;

    virtual bool ToZombie(Thread *_Thread) = 0;

    virtual States Type() = 0;
};

class ThreadStateUninterruptable : public ThreadState {
public:
    virtual bool ToRun(Thread *_Thread) override;

    virtual bool ToIOBlocked(Thread *_Thread) override;

    virtual bool ToPause(Thread *_Thread) override;

    virtual bool ToReady(Thread *_Thread) override;

    virtual bool ToZombie(Thread *_Thread) override;

    virtual States Type() override;
};

class ThreadStateInterruptable : public ThreadState {
public:
    virtual bool ToRun(Thread *_Thread) override;

    virtual bool ToIOBlocked(Thread *_Thread) override;

    virtual bool ToPause(Thread *_Thread) override;

    virtual bool ToReady(Thread *_Thread) override;

    virtual bool ToZombie(Thread *_Thread) override;

    virtual States Type() override;
};

class ThreadStateReady : public ThreadState {
public:
    virtual bool ToRun(Thread *_Thread) override;

    virtual bool ToIOBlocked(Thread *_Thread) override;

    virtual bool ToPause(Thread *_Thread) override;

    virtual bool ToReady(Thread *_Thread) override;

    virtual bool ToZombie(Thread *_Thread) override;

    virtual States Type() override;
};

class ThreadStateZombie : public ThreadState {
public:
    virtual bool ToRun(Thread *_Thread) override;

    virtual bool ToIOBlocked(Thread *_Thread) override;

    virtual bool ToPause(Thread *_Thread) override;

    virtual bool ToReady(Thread *_Thread) override;

    virtual bool ToZombie(Thread *_Thread) override;

    virtual States Type() override;
};

class ThreadStateRunning : public ThreadState {
public:
    virtual bool ToRun(Thread *_Thread) override;

    virtual bool ToIOBlocked(Thread *_Thread) override;

    virtual bool ToPause(Thread *_Thread) override;

    virtual bool ToReady(Thread *_Thread) override;

    virtual bool ToZombie(Thread *_Thread) override;

    virtual States Type() override;
};
