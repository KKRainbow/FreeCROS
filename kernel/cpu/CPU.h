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

#pragma once

class Thread;
class CPU
{
protected:
	friend class CPUManager;
	addr_t kernelStackAddr;
	size_t kernelStackSize;
	int id;
public:
	enum Type{BSP,AP,UNKNOWN}type;
	virtual void InitAsBSP() = 0;
	virtual void InitAsAP(addr_t _Stack,size_t _StackSize) = 0;
	virtual void Run() = 0;
	virtual ~CPU(){};
	virtual Thread* GetCurrThreadRunning() = 0;
	virtual Type GetType() = 0;
	virtual void StartService() = 0;
	virtual void ExhaustCurrThread() = 0;
	void SetID(int _Id){id = _Id;}
	int GetID()const{return id;}
};
