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

#include "HeapManager.h"
#include "cpu/CPUManager.h"
#include "thread/Thread.h"

SINGLETON_CPP(HeapManager) {

}

/**
 * @brief 找到当前线程所对应的堆
 */
Heap *HeapManager::GetHeapOfThisThread() {
    auto curr = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
    return this->GetHeapOfThread(curr);
}

void HeapManager::CreateHeapForThread(Thread *_Thread) {
    while ( _Thread->belongTo != nullptr ) _Thread = _Thread->belongTo;
    if ( GetHeapOfThread(_Thread) == nullptr ) {
        this->heapmap.Insert(lr::sstl::MakePair(_Thread->GetPid(), new Heap(_Thread)));
    }
}

Heap *HeapManager::GetHeapOfThread(Thread *_Thread) {
    while ( _Thread->belongTo != nullptr ) _Thread = _Thread->belongTo;
    auto itr = heapmap.Find(_Thread->GetPid());
    return itr == heapmap.End() ? nullptr : itr->second.Obj();
}
