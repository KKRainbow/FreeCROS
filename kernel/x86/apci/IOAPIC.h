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
#include"Global.h"

class IOAPIC
{
	private:
		addr_t addr;
		int base;
		int count;
		void WriteToReg(int _Reg,uint32_t _Data);
		uint32_t ReadReg(int _Reg);
	public:
		IOAPIC(addr_t _Addr);
		void ChangeGlobalIntrBase(int _Base,int _LAPIC);
		void Unmask(int i);
		void Mask(int i);
		int GetIRQCount();
};
