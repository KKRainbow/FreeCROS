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

#include"RamDiskItemFile.h"

RamDiskItemFile::RamDiskItemFile(int32_t _Id,Type _Type,lr::sstl::AString _Name)
:RamDiskItem(nullptr,_Id,_Type,_Name)
{

}

pid_t RamDiskItemFile::Seek(File *_Fptr, off_t _Offset, int _Whence)
{
	return 0;
}

pid_t RamDiskItemFile::Write(File *_Fptr, int8_t *_Buffer, size_t _Size)
{
	return 0;
}

pid_t RamDiskItemFile::Read(File *_Fptr, int8_t *_Buffer, size_t _Size)
{
	return 0;
}

pid_t RamDiskItemFile::Open()
{
	return 0;
}
