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
#include"thread/IPCMessage.h"
#include"Device.h"


RamDiskItemChrDev::RamDiskItemChrDev
(Thread* _Thread,int32_t _Id,Type _Type,lr::sstl::AString _Name)
:RamDiskItem(_Id,_Type,_Name)
{
	Assert(_Thread);
	this->pid = _Thread->GetPid();
}

Thread* RamDiskItemChrDev::BuildMsg(Message& _Msg)
{
	auto currThread = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	auto devThread = ThreadManager::Instance()->GetThreadByPID(this->pid);
	Assert(currThread);
	if(!devThread)return nullptr;
	
	_Msg.source = currThread->GetPid();
	_Msg.destination = devThread->GetPid();
	_Msg.timeStamp = CPUManager::Instance()->GetClockCounter();
	
	return devThread;
}
pid_t RamDiskItemChrDev::Open()
{
	Message msg;
	auto devThread = this->BuildMsg(msg);
	if(!devThread)
	{
		return -1;
	}
	msg.content[0] = MSG_DEVICE_OPEN_OPERATION;
	
	IPCMessage ipc = msg;
	
	if(devThread->ReceiveMessage(ipc))
	{
		devThread->waitIPCReceive.Wake();
		return devThread->GetPid();
	}
	else
	{
		return -1;
	}
}
pid_t RamDiskItemChrDev::Read(int8_t* _Buffer,size_t _Size)
{
	Message msg;
	auto devThread = this->BuildMsg(msg);
	if(!devThread)
	{
		return -1;
	}
	msg.content[0] = MSG_DEVICE_READ_OPERATION;
	msg.content[1] = (uint32_t)_Buffer;
	msg.content[2] = (uint32_t)_Size;
	IPCMessage ipc = msg;
	if(devThread->ReceiveMessage(ipc))
	{
		devThread->waitIPCReceive.Wake();
		return devThread->GetPid();
	}
	else
	{
		return -1;
	}
}
pid_t RamDiskItemChrDev::Write(int8_t* _Buffer,size_t _Size)
{
	Message msg;
	auto devThread = this->BuildMsg(msg);
	if(!devThread)
	{
		return -1;
	}
	msg.content[0] = MSG_DEVICE_OPEN_OPERATION;
	msg.content[1] = (uint32_t)_Buffer;
	msg.content[2] = (uint32_t)_Size;
	IPCMessage ipc = msg;
	if(devThread->ReceiveMessage(ipc))
	{
		devThread->waitIPCReceive.Wake();
		return devThread->GetPid();
	}
	else
	{
		return -1;
	}
}
pid_t RamDiskItemChrDev::Seek(off_t _Offset,int _Whence)
{
	Message msg;
	auto devThread = this->BuildMsg(msg);
	if(!devThread)
	{
		return -1;
	}
	msg.content[0] = MSG_DEVICE_OPEN_OPERATION;
	msg.content[1] = _Offset;
	msg.content[2] = _Whence;
	IPCMessage ipc = msg;
	if(devThread->ReceiveMessage(ipc))
	{
		devThread->waitIPCReceive.Wake();
		return devThread->GetPid();
	}
	else
	{
		return -1;
	}
}