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
#include"stl/smap.h"
#include"stl/stuple.h"
#include"stl/sstring.h"

typedef int32_t IDType;
class RamDiskItem
{
public:
	friend class RamDisk;
	enum Type{
		FILE,
		DIR,
		BLOCK,
		CHAR
	};
private:
	typedef lr::sstl::Map<lr::sstl::AString,RamDiskItem*> ItemList;
	Type type;
	RamDiskItem* parent = nullptr;//暂不支持硬链接,为DAG
	IDType id; //每个文件唯一一个ID
	ItemList children;
	lr::sstl::AString name;
	RamDiskItem(int32_t _Id,Type _Type,lr::sstl::AString _Name);
	IDType AddChild(RamDiskItem* _Child);
public:
	Type GetType();
	lr::sstl::Pair<ItemList::iterator,ItemList::iterator> GetChildrenIter();
	RamDiskItem* GetParent();
	IDType GetID();
	RamDiskItem* FindChildByName(lr::sstl::AString _Name);
	lr::sstl::AString GetName();
public:
// 	virtual int Open();
// 	virtual size_t Read();
// 	virtual size_t Write();
// 	virtual int Seek();
};
