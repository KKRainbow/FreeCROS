#pragma once 

#include"Scheduler.h"
#include"ThreadManager.h"
#include"cpu/CPUManager.h"
#include"stl/slinkedlist.h"
#include"SpinLock.h"


class SchedulerDefault : public Scheduler {
private:
    static const int MAX_LIST_SIZE = 10;
    lr::sstl::List<Thread *> lists[MAX_LIST_SIZE];
    SpinLock lock;
public:
    virtual void Init() override;

    virtual void Deinit() override;

    virtual void ThreadAdded(Thread *thread) override;

    virtual void ThreadRemoved(Thread *thread) override;

    virtual Thread *NextThread(CPU *_CPU) override;

    virtual void RaisePriority(Thread *thread, int _Pri) override;

    virtual void DownPriority(Thread *thread, int _Pri) override;
};

