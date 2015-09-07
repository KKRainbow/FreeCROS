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

#include"RamDisk.h"

#include"stl/svector.h"
#include "RamDiskItemMounted.h"
#include"RamDiskItemChrDev.h"
#include"RamDiskItemDir.h"
#include"RamDiskItemFile.h"
#include"RamDiskItemKernel.h"
#include <cpu/CPUManager.h>
#include <fsserver/format.h>

using namespace lr::sstl;

SINGLETON_CPP(RamDisk) {
    this->root = this->GetNewItem("root", RamDiskItem::Type::DIR);
    Assert(this->root);

    //创建一些必须的目录
    this->MakeDir("dev");
    this->MakeDir("etc");
    this->MakeDir("home");

}

RamDiskItem *RamDisk::GetItemByID(IDType _Id) {
    auto i = this->itemsMap.Find(_Id);
    return i == this->itemsMap.End() ? nullptr : i->second;
}

bool RamDisk::RemoveItemByID(IDType _Id) {
    return this->RemoveItemByPtr(GetItemByID(_Id));
}

bool RamDisk::RemoveItemByPtr(RamDiskItem *_Ptr) {
    if ( _Ptr == nullptr )return true;
    auto ites = _Ptr->GetChildrenIter();
    auto &begin = ites.first;
    auto &end = ites.second;
    for ( ; begin != end; ++begin ) {
        RemoveItemByPtr(begin->second);
    }
    this->itemsMap.Erase(_Ptr->GetID());
    return true;
}

RamDiskItem *RamDisk::GetItemByPath(AString _Path, RamDiskItem *_Root) {
    if ( _Path == "/" )return this->root;
    Vector<AString> names;
    _Path.Split(names, '/');
    if ( names.Size() == 0 )return nullptr;
    if ( _Root == nullptr )_Root = this->root;
    _Path = _Path.Trim('/');
    auto absolutePath = this->GetAbsolutePath(_Root) + "/" + _Path;
    for ( auto &name : names ) {
        _Root = _Root->FindChildByName(name);
        if ( _Root == nullptr )return nullptr;
        if(_Root->GetType() == RamDiskItem::DIR)
        {
            RamDiskItemDir* dir = (RamDiskItemDir*)_Root;
            if(dir->IsMounted())
            {
                //创建对应的mounted item
                auto mounted = new RamDiskItemMounted(dir,-1,idgen.GetID(),
                absolutePath,nullptr);
                this->itemsMap.Insert(MakePair(mounted->GetID(),mounted));
                return mounted;
            }
        }
    }
    return _Root;
}

RamDiskItem *RamDisk::RegisterBlockDevice(AString _Name) {
    return nullptr;
}

RamDiskItem *RamDisk::RegisterCharaterDevice(AString _Name) {
    auto ri = this->GetNewItem(_Name, RamDiskItem::Type::CHAR);
    Assert(ri);
    auto dev = this->GetItemByPath("/dev");
    if ( dev == nullptr ) {
        dev = this->GetItemByID(this->MakeDir("dev", this->root));
        Assert(dev);
    }
    LOG("DEV ADDR: 0x%x\n", dev);
    dev->AddChild(ri);
    return ri;
}

RamDiskItem *RamDisk::GetNewItem(AString _Name, RamDiskItem::Type _Type) {
    if ( _Name.Length() == 0 ) {
        return nullptr;
    }
    RamDiskItem *i = nullptr;
    auto currThread = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
    switch ( _Type ) {
        case RamDiskItem::Type::CHAR:
            i = new RamDiskItemChrDev
                    (currThread, this->idgen.GetID(), _Type, _Name);
            break;
        case RamDiskItem::Type::DIR:
            i = new RamDiskItemDir(this->idgen.GetID(),  _Name);
            break;
        case RamDiskItem::Type::FILE:
            i = new RamDiskItemFile(this->idgen.GetID(), _Type, _Name);
            break;
    }
    if ( i )this->itemsMap.Insert(MakePair(i->id, i));
    return i;
}

IDType RamDisk::MakeDir(lr::sstl::AString _Path, RamDiskItem *_Parent, bool _Recursive) {
    Vector<AString> names;
    _Path = _Path.Trim('/');
    auto file = BaseName(_Path);
    auto pwd = DirName(_Path);
    if ( _Parent == nullptr )_Parent = this->root;
    AString absolutePath = this->GetAbsolutePath(_Parent) + "/" + _Path;
    if ( _Recursive == false ) {
        if ( pwd == "." ) {
            //当前目录就是要创建目录的地方
            auto id = -1;
            auto newItem = this->GetNewItem(file, RamDiskItem::Type::DIR);
            id = _Parent->AddChild(newItem);
            if ( id < 0 )delete newItem;
            return id;
        }
        else {
            _Parent = this->GetItemByPath(pwd, _Parent);
            if ( _Parent == nullptr )return -1;
            if ( _Parent->GetType() != RamDiskItem::DIR )return -2;
            return this->MakeDir(file, _Parent);
        }
    }
    else {
        Vector<AString> pwdnames;
        pwd.Split(pwdnames, '/');
        for ( auto &i : pwdnames ) {
            auto child = _Parent->FindChildByName(i);
            if ( child == nullptr ) {
                auto id = this->MakeDir(i, _Parent);
                Assert(id >= 0);
                Assert(_Parent = this->GetItemByID(id));
            }
            else {
                if ( child->GetType() != RamDiskItem::DIR ) {
                    return -2;
                }
                auto dirItem = (RamDiskItemDir*)child;
                if (dirItem->IsMounted())
                {
                    //创建对应的mounted item
                    LOG("Mkdir get mounted dir\n");
                    auto mounted = new RamDiskItemMounted(dirItem,-1,idgen.GetID(),
                                                          absolutePath,nullptr);
                    if (mounted->Mkdir(0777,_Recursive) > 0)
                    {
                        this->itemsMap.Insert(MakePair(mounted->GetID(),mounted));
                        return mounted->GetID();
                    }
                    return -1;
                }
                _Parent = child;
            }
        }
        return this->MakeDir(file, _Parent);
    }

}

IDType RamDisk::CreateFile(lr::sstl::AString _Name, RamDiskItem *_Parent) {
    if ( _Parent == nullptr )_Parent = this->root;
    if ( _Parent->GetType() != RamDiskItem::Type::DIR )return -1;
    //TODO 检查_Parent是不是归我们管理?
    auto newItem = this->GetNewItem(_Name, RamDiskItem::Type::FILE);
    auto id = _Parent->AddChild(newItem);
    if ( id < 0 )delete newItem;
    return id;
}

IDType RamDisk::CreateKernelDev(RamDiskItemKernel *_Item) {
    Assert(_Item);
    RamDiskItem *dev = this->GetItemByPath("/dev");
    if ( dev == nullptr ) {
        dev = this->GetItemByID(this->MakeDir("dev", this->root));
        Assert(dev);
    }
    _Item->id = this->idgen.GetID();
    this->itemsMap.Insert(MakePair(_Item->id, _Item));
    return dev->AddChild(_Item);
}

lr::sstl::AString RamDisk::GetAbsolutePath(RamDiskItem* _Dir) {
    if ( _Dir->GetType() == RamDiskItem::MOUNTED)
    {
        return _Dir->GetName();
    }
    if ( _Dir->GetType() == RamDiskItem::DIR)
    {
        if (_Dir == this->root)
        {
            return AString::Empty;
        }
        return this->GetAbsolutePath((RamDiskItemDir*)_Dir->GetParent()) + "/" +
               _Dir->GetName();
    }
    return AString::Empty;
}
