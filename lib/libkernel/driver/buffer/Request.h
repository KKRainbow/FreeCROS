//
// Created by ssj on 15-8-13.
//
#pragma once

#include "Global.h"
#include"Buffer.h"

struct Req{
    enum Cmd{READ,WRITE,READA<WRITEA};
    int dev;		/* -1 if no request */
    int cmd;		/* READ or WRITE */
    int errors;
    unsigned long sector;
    unsigned long nr_sectors;
    char * buffer;
    pid_t thread_waiting;
    Buffer * bh;
    Req* next;
};
class Request {
private:
    int req_size;
    Req* req_head;
    bool waiting;
    BlkDevStruct* dev_struct;
    SpinLock lock;
    void AddRequest(Req* _Req);
public:
    Request(int req_size,BlkDevStruct* _Dev);
    void MakeRequest(Req::Cmd _Cmd,Buffer* _Buf,pid_t _Pid);
    void MakeRequest(Req::Cmd _Cmd,int _Sector,char* buffer,int _Size,pid_t _Pid);
    void EndRequest(bool Uptodate);
};


