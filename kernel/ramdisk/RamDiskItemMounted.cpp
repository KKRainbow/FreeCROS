//
// Created by ssj on 15-9-6.
//

#include "RamDiskItemMounted.h"
#include <SystemCalls.h>
#include <fsserver/format.h>
#include <Device.h>
#include "thread/Thread.h"

pid_t RamDiskItemMounted::Open() {
    char nameStr[1024];
    this->GetName().CStr(nameStr);
    char modeStr[10];
    modeStr[0] = 0;
//    mode.CStr(modeStr);
    ino_t rootInode = this->rootItem ? rootItem->internalId : 0;

    pid_t pid = this->thread->GetPid();
    Message msg;
    msg.destination = pid;
    msg.content[0] = MSG_DEVICE_OPEN_OPERATION;
    msg.content[FsMsg::O_PATH] = (uint32_t)nameStr;
    msg.content[FsMsg::O_PATH_SIZE] = (uint32_t)strlen(nameStr);
    msg.content[FsMsg::O_MODE] = (uint32_t)modeStr;
    msg.content[FsMsg::O_MODE_SIZE] = (uint32_t)strlen(modeStr);
    msg.content[FsMsg::O_ROOTID] = (uint32_t)rootInode;
    SysCallSendMessageTo::Invoke((uint32_t)&msg);
    SysCallReceiveFrom::Invoke((uint32_t) pid, (uint32_t) &msg, 0, 0);

    this->internalId = msg.content[0];
    return msg.content[0];
}

pid_t RamDiskItemMounted::Read(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    if(this->internalId < 0)return -1;
    Message msg;
    char* buffer = new char[_Size];
    msg.content[0] = MSG_DEVICE_READ_OPERATION;
    msg.content[FsMsg::R_FD] = (uint32_t)this->internalId;
    msg.content[FsMsg::R_BUF] = (uint32_t)buffer;
    msg.content[FsMsg::R_POS] = (uint32_t)_Fptr->f_pos;
    msg.content[FsMsg::R_SIZE] = (uint32_t)_Size;
    pid_t pid = this->thread->GetPid();
    msg.destination = pid;
    SysCallSendMessageTo::Invoke((uint32_t)&msg);
    SysCallReceiveFrom::Invoke((uint32_t) pid, (uint32_t) &msg, 0, 0);
    memcpy(_Buffer, buffer, msg.content[0]);
    delete buffer;
    return msg.content[0];
    return 0;
}

pid_t RamDiskItemMounted::Write(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    if(this->internalId < 0)return -1;
    Message msg;
    msg.content[0] = MSG_DEVICE_WRITE_OPERATION;
    msg.content[FsMsg::W_FD] = (uint32_t)this->internalId;
    msg.content[FsMsg::W_BUF] = (uint32_t)_Buffer;
    msg.content[FsMsg::W_POS] = (uint32_t)_Fptr->f_pos;
    msg.content[FsMsg::W_SIZE] = (uint32_t)_Size;
    pid_t pid = this->thread->GetPid();
    msg.destination = pid;
    SysCallSendMessageTo::Invoke((uint32_t)&msg);
    SysCallReceiveFrom::Invoke((uint32_t) pid, (uint32_t) &msg, 0, 0);
    return msg.content[0];
}

pid_t RamDiskItemMounted::Seek(File *_Fptr, off_t _Offset, int _Whence) {
    return 0;
}

RamDiskItemMounted::RamDiskItemMounted(RamDiskItem* _Mounted, ino_t _Inner, IDType _Id,lr::sstl::AString _Path,
        RamDiskItemMounted* _Root)
:RamDiskItem(nullptr,_Id,RamDiskItem::MOUNTED,_Path),rootItem(_Root)
{
    this->mountPoint = (RamDiskItemDir*)_Mounted;
    this->thread = _Mounted->GetThread();
}

int RamDiskItemMounted::Mkdir(int mode, bool _Recursive) {
    Message msg;
    char nameStr[1024];
    this->GetName().CStr(nameStr);
    msg.content[0] = MSG_DEVICE_MKDIR_OPERATION;
    msg.content[FsMsg::M_PATH] = (uint32_t)nameStr;
    msg.content[FsMsg::M_PATH_SIZE] = (uint32_t)strlen(nameStr);
    msg.content[FsMsg::M_MODE] = (uint32_t)this->internalId;
    msg.content[FsMsg::M_RECURSIVE] = (uint32_t)_Recursive;
    pid_t pid = this->thread->GetPid();
    msg.destination = pid;
    SysCallSendMessageTo::Invoke((uint32_t)&msg);
    SysCallReceiveFrom::Invoke((uint32_t) pid, (uint32_t) &msg, 0, 0);

    this->internalId = msg.content[0];
    return msg.content[0];
}
