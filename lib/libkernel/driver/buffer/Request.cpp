//
// Created by ssj on 15-8-13.
//

#include <SystemCalls.h>
#include "Request.h"

#define IN_ORDER(s1,s2) \
((s1)->cmd<(s2)->cmd || (s1)->cmd==(s2)->cmd && \
(((s1)->sector < (s2)->sector)))

void Request::AddRequest(Req * _Req) {
    Req* tmp;
    Req* req = _Req;

    req->next = nullptr;
    if (req->bh)
        req->bh->b_dirt = 0;
    if (!(tmp = this->current_req)) {
        this->current_req = req;
        this->CallRequestFunc();
        return;
    }
    for ( ; tmp->next ; tmp=tmp->next) {
        if (!req->bh)
        if (tmp->next->bh)
            break;
        else
            continue;
        if ((IN_ORDER(tmp,req) ||
             !IN_ORDER(tmp,tmp->next)) &&
            IN_ORDER(req,tmp->next))
            break;
    }
    req->next=tmp->next;
    tmp->next=req;
}


void Request::MakeRequest(Req::Cmd _Cmd, Buffer *_Buf, pid_t _Pid) {
    Req* req;
    int rw_ahead;
    Req::Cmd rw = _Cmd;
    Buffer* bh = _Buf;

/* WRITEA/READA is special case - it is not really needed, so if the */
/* buffer is locked, we just forget about it, else it's a normal read */
    //TODO 预读调整在内核中判断
    rw_ahead = (rw == Req::Cmd:: READA || rw == Req::Cmd::WRITEA);
//    if ((rw_ahead = (rw == Req::Cmd:: READA || rw == Req::Cmd::WRITEA))) {
//        if (bh->b_lock)
//            return;
//        if (rw == READA)
//            rw = READ;
//        else
//            rw = WRITE;
//    }
    if (rw!=Req::READ && rw!=Req::WRITE)
    {
        return;//Assert("Bad block dev command, must be R/W/RA/WA");
    }
    if ((rw == Req::WRITE && !bh->b_dirt) || (rw == Req::READ && bh->b_uptodate)) {
        return;
    }
    repeat:
/* we don't allow the write-requests to fill up the queue completely:
 * we want some room for reads: they take precedence. The last third
 * of the requests are only for reads.
 */
    if (rw == Req::READ)
        req = this->req_head+this->req_size;
    else
        req = this->req_head+((this->req_size*2)/3);
    lock.Lock();
/* find an empty request */
    while (--req >= this->req_head)
        if (req->dev < 0)
            break;
/* if none found, sleep on new requests: check for rw_ahead */
    if (req < this->req_head) {
        lock.Unlock();
        if (rw_ahead) {
            return;
        }
//        sleep_on(&wait_for_request);
        waiting = true;
        while(waiting)
        {
            SysCallGiveUp::Invoke();
        }
        goto repeat;
    }
/* fill up the request-info, and add it to the queue */
    req->dev = bh->b_dev;
    req->cmd = rw;
    req->errors=0;
    req->sector = bh->b_blocknr<<1;
    req->nr_sectors = 2;
    req->buffer = (char*)bh->b_data;
    req->thread_waiting = _Pid;
    req->bh = bh;
    req->next = nullptr;
    AddRequest(req);
    lock.Unlock();
}

void Request::MakeRequest(Req::Cmd _Cmd, int _Devnum, int _Sector, char *buffer, int _Count, pid_t _Pid) {
    Req* req;

    if (_Cmd!=Req::READ && _Cmd!=Req::WRITE)
    {
        return;//panic("Bad block dev command, must be R/W");
    }
    repeat:
    lock.Lock();
    req = req_head+req_size;
    while (--req >= req_head)
        if (req->dev < 0)
            break;
    if (req < req_head) {
        lock.Unlock();
        waiting = true;
        while(waiting)
        {
            SysCallGiveUp::Invoke();
        }
        goto repeat;
    }
/* fill up the request-info, and add it to the queue */
    req->dev = _Devnum;
    req->cmd = _Cmd;
    req->errors = 0;
    req->sector = (unsigned long)_Sector;
    req->nr_sectors = (unsigned int)_Count;
    req->buffer = buffer;
    req->thread_waiting = _Pid;
    req->bh = nullptr;
    req->next = nullptr;
    AddRequest(req);
    lock.Unlock();
}

void Request::EndRequest(bool Uptodate) {
    if (this->current_req->bh) {
        this->current_req->bh->b_uptodate = Uptodate;
        this->current_req->bh->UnlockBuffer();
    }
    if (!Uptodate) {
//        printk(DEVICE_NAME " I/O error\n\r");
//        printk("dev %04x, block %d\n\r",this->current_req->dev,
//               this->current_req->bh->b_blocknr);
    }
    this->waiting = false;
    this->current_req->dev = -1;
    this->current_req = this->current_req->next;
}

Req *Request::GetCurrentReq() {
    return this->current_req;
}

Request::Request(int _Size, void (*_Func)(Request*)) : req_size(_Size),request_fn(_Func) {
    this->req_head = new Req[req_size];
}

