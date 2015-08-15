//
// Created by ssj on 15-8-13.
//

#include <drivers/hd/blk.h>
#include "Request.h"

void Request::MakeRequest() {

}

void Request::AddRequest(Req * _Req) {
    Req* tmp;
    Req* req = _Req;
    BlkDevStruct* dev = this->dev_struct;

    req->next = NULL;
    if (req->bh)
        req->bh->b_dirt = 0;
    if (!(tmp = dev->current_request)) {
        dev->current_request = req;
        (dev->request_fn)();
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


Request::Request(int _Size,BlkDevStruct* _Dev) :req_size(_Size),dev_struct(_Dev){
    this->req_head = new Req[req_size];
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
        if (req->dev<0)
            break;
/* if none found, sleep on new requests: check for rw_ahead */
    if (req < this->req_head) {
        if (rw_ahead) {
            lock.Unlock();
            return;
        }
//        sleep_on(&wait_for_request);
        waiting = true;
        while(waiting);
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
    req->next = NULL;
    AddRequest(req);
    lock.Unlock();
}

void Request::MakeRequest(Req::Cmd _Cmd, int _Sector, char *buffer, int _Size, pid_t _Pid) {
    Req* req;

    if (_Cmd!=Req::READ && _Cmd!=Req::WRITE)
    {
        return;//panic("Bad block dev command, must be R/W");
    }
    repeat:
    req = req_head+req_size;
    while (--req >= req_head)
        if (req->dev<0)
            break;
    if (req < req_head) {
        waiting = true;
        while(waiting);
        goto repeat;
    }
/* fill up the request-info, and add it to the queue */
    req->dev = dev_struct->dev_num;
    req->cmd = _Cmd;
    req->errors = 0;
    req->sector = (unsigned long)_Sector;
    req->nr_sectors = 8;
    req->buffer = buffer;
    req->thread_waiting = _Pid;
    req->bh = nullptr;
    req->next = nullptr;
    AddRequest(req);
}

void Request::EndRequest(bool Uptodate) {
    if (this->dev_struct->current_request->bh) {
        this->dev_struct->current_request->bh->b_uptodate = Uptodate;
    }
    if (!Uptodate) {
//        printk(DEVICE_NAME " I/O error\n\r");
//        printk("dev %04x, block %d\n\r",this->dev_struct->current_request->dev,
//               this->dev_struct->current_request->bh->b_blocknr);
    }
    this->waiting = false;
    this->dev_struct->current_request->dev = -1;
    this->dev_struct->current_request = this->dev_struct->current_request->next;
}
