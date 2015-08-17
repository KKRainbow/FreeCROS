/*
 * Copyright 2015 <copyright holder> <email>
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#include "RamDiskItemChrDev.h"
#include <cpu/CPUManager.h>
#include <thread/ThreadManager.h>
#include"Device.h"


RamDiskItemChrDev::RamDiskItemChrDev
        (Thread *_Thread, int32_t _Id, Type _Type, lr::sstl::AString _Name)
        : RamDiskItem(_Thread, _Id, _Type, _Name) {
}

Thread *RamDiskItemChrDev::BuildMsg(Message &_Msg) {
    auto currThread = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
    auto devThread = this->thread;
    Assert(currThread);
    if ( !devThread )return nullptr;

    _Msg.source = currThread->GetPid();
    _Msg.destination = devThread->GetPid();
    _Msg.timeStamp = CPUManager::Instance()->GetClockCounter();

    return devThread;
}

pid_t RamDiskItemChrDev::Open() {
    Message msg;
    auto devThread = this->BuildMsg(msg);
    if ( !devThread ) {
        return -1;
    }
    msg.content[0] = MSG_DEVICE_OPEN_OPERATION;

    IPCMessage ipc = msg;

    if ( devThread->ReceiveMessage(ipc)) {
        devThread->waitIPCReceive.Wake();
        auto pid = devThread->GetPid();
        //获取的是设备进程的pid,要想获取最终结果还需ReceiMsg一下
        SysCallReceiveFrom::Invoke((uint32_t) pid, (uint32_t) &msg, 0, 0);
        return this->GetID();
    }
    else {
        return -1;
    }
}

pid_t RamDiskItemChrDev::Read(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    Message msg;
    auto devThread = this->BuildMsg(msg);
    if ( !devThread ) {
        return -1;
    }
    msg.content[0] = MSG_DEVICE_READ_OPERATION;
    msg.content[1] = (uint32_t) _Buffer;
    msg.content[2] = (uint32_t) _Size;
    IPCMessage ipc = msg;
    if ( devThread->ReceiveMessage(ipc)) {
        devThread->waitIPCReceive.Wake();
        auto pid = devThread->GetPid();
        SysCallReceiveFrom::Invoke((uint32_t) pid, (uint32_t) &msg, 0, 0);
        //参数: data,size;
        if ( msg.content[0] == 0 )return -1;
        ThreadManager::TransferDateFromOtherThread((void *) _Buffer, devThread,
                                                   (void *) msg.content[0], msg.content[1]
        );
        return msg.content[1];
    }
    else {
        return -1;
    }
}

pid_t RamDiskItemChrDev::Write(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    Message msg;
    auto devThread = this->BuildMsg(msg);
    if ( !devThread ) {
        return -1;
    }
    msg.content[0] = MSG_DEVICE_OPEN_OPERATION;
    msg.content[1] = (uint32_t) _Buffer;
    msg.content[2] = (uint32_t) _Size;
    IPCMessage ipc = msg;
    if ( devThread->ReceiveMessage(ipc)) {
        devThread->waitIPCReceive.Wake();
        return devThread->GetPid();
    }
    else {
        return -1;
    }
}

pid_t RamDiskItemChrDev::Seek(File *_Fptr, off_t _Offset, int _Whence) {
    Message msg;
    auto devThread = this->BuildMsg(msg);
    if ( !devThread ) {
        return -1;
    }
    msg.content[0] = MSG_DEVICE_OPEN_OPERATION;
    msg.content[1] = _Offset;
    msg.content[2] = _Whence;
    IPCMessage ipc = msg;
    if ( devThread->ReceiveMessage(ipc)) {
        devThread->waitIPCReceive.Wake();
        return devThread->GetPid();
    }
    else {
        return -1;
    }
}