#pragma once

#include"Global.h"
#include"Threads.h"

class IPCMessage {
public:
    Message msg;

    pid_t GetSource();

    pid_t GetDestination();

    IPCMessage(Message msg);

    IPCMessage() { }

    uint64_t GetTimeStamp();

    bool operator<(const IPCMessage &msg);

    bool operator<=(const IPCMessage &msg);

    bool operator>(const IPCMessage &msg);

    bool operator>=(const IPCMessage &msg);
};