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

#include"RamDiskItem.h"
#include"stl/sidgen.h"
#include"stl/smap.h"
#include"stl/sstring.h"
#include"stl/stuple.h"

class RamDiskItemKernel;

class RamDisk {
SINGLETON_H(RamDisk)

private:
    lr::sstl::Map<IDType, RamDiskItem *> itemsMap;
    RamDiskItem *root = nullptr;
    lr::sstl::IDGenerator<IDType> idgen;

    RamDiskItem *GetNewItem(lr::sstl::AString _Name, RamDiskItem::Type _Type);

public:
    const static IDType INVALIDATE_ID = -1;

    RamDiskItem *GetItemByID(IDType _Id);

    bool RemoveItemByID(IDType _Id);

    bool RemoveItemByPtr(RamDiskItem *_Ptr);

    RamDiskItem *GetItemByPath(lr::sstl::AString _Path, RamDiskItem *_Root = nullptr);

    RamDiskItem *RegisterBlockDevice(lr::sstl::AString _Name);

    RamDiskItem *RegisterCharaterDevice(lr::sstl::AString _Name);

    IDType MakeDir(lr::sstl::AString _Path, RamDiskItem *_Parent = nullptr, bool _Recursive = false);

    IDType CreateFile(lr::sstl::AString _Name, RamDiskItem *_Parent = nullptr);

    IDType CreateKernelDev(RamDiskItemKernel *_Item);

    lr::sstl::AString GetAbsolutePath(RamDiskItem* _Dir);
};
