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

#include "Multiboot.h"
#include "string.h"

Multiboot::Multiboot()
{
	memcpy(this->intTable, NULL, sizeof(intTable));
}
Multiboot::Multiboot(const Multiboot& _x):InfoTable(_x.InfoTable)
{
	memcpy(this->intTable, _x.intTable, sizeof(intTable));
}
bool Multiboot::TestFlagVBE() { return bitmap[11] == 1;}
bool Multiboot::TestFlagAPM() { return bitmap[10] == 1;}
bool Multiboot::TestFlagName() { return bitmap[9] == 1;}
bool Multiboot::TestFlagConfig() { return bitmap[8] == 1;}
bool Multiboot::TestFlagDrives() { return bitmap[7] == 1;}
bool Multiboot::TestFlagMmap() { return bitmap[6] == 1;}
bool Multiboot::TestFlagSyms() { return bitmap[4] == 1||bitmap[5]==1;}
bool Multiboot::TestFlagMods() { return bitmap[3] == 1;}
bool Multiboot::TestFlagCmd() { return bitmap[2] == 1;}
bool Multiboot::TestFlagBootDev() { return bitmap[1] == 1; }
bool Multiboot::TestFlagMem() { return bitmap[0] == 1; }
const MultibootInfo& Multiboot::GetMultibootInfo()
{
	return this->InfoTable;
}
void Multiboot::SetInfoAddr(MultibootInfo* _Addr)
{
	this->InfoTable = *_Addr;
	this->bitmap = lr::sstl::Bitmap<unsigned char,1>(reinterpret_cast<unsigned char*>(&(this->InfoTable.flags)),32,false);
}

char *Multiboot::GetIntTable() {
	return this->intTable;
}
