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

#include <fsserver/format.h>
#include <thread/IPCMessage.h>
#include <SystemCalls.h>
#include "RamDiskItemDir.h"
#include "thread/Thread.h"
#include "Device.h"
#include "dirent.h"

RamDiskItemDir::RamDiskItemDir(int32_t _Id, lr::sstl::AString _Name)
        : RamDiskItem(nullptr, _Id, Type::DIR, _Name) {

}

pid_t RamDiskItemDir::Seek(File *_Fptr, off_t _Offset, int _Whence) {
    return 1;
}

pid_t RamDiskItemDir::Write(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    //目录不能写
    return -1;
}

pid_t RamDiskItemDir::Read(File *_Fptr, int8_t *_Buffer, size_t _Size) {
    if (!_Fptr)
    {
        return -1;
    }
    if (this->IsMounted() || (_Size % sizeof(dirent) != 0 || _Fptr->f_pos % sizeof(dirent) != 0))
    {
        return -1;
    }
    int offset = _Fptr->f_pos / sizeof(dirent);
    int count = _Size / sizeof(dirent);
    dirent* buf = (dirent*)_Buffer;
    auto tmp = this->children.Begin();
    for (int i = 0;i< offset && tmp != this->children.End();i++,tmp++);
    if (this->children.End() == tmp) return 0;

    for ( int i = 0;i < count ;i++)
    {
        dirent dir;
        dir.d_ino = this->GetID();
        //TODO 检查长度是否超过
        this->GetName().CStr(dir.d_name);
        dir.d_type = this->GetType();
        dir.d_reclen = 0; //not supported

        *buf++ = dir;
        ++tmp;
        if(this->children.End() == tmp)
        {
            return i * sizeof(dirent);
        }
    }
    return _Size;
}

pid_t RamDiskItemDir::Open() {
    if (this->thread)
    {

    }
    return 1;
}

void RamDiskItemDir::Mount(Thread *_Thread) {
    this->thread = _Thread;
}

