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

#include"RamDiskItem.h"

using namespace lr::sstl;

RamDiskItem::Type RamDiskItem::GetType() {
    return this->type;
}

RamDiskItem::RamDiskItem(Thread *_Thread, int32_t _Id, Type _Type, AString _Name)
        : id(_Id), type(_Type), name(_Name), thread(_Thread) {

}

lr::sstl::Pair<RamDiskItem::ItemList::iterator, RamDiskItem::ItemList::iterator>
RamDiskItem::GetChildrenIter() {
    return MakePair(this->children.Begin(), this->children.End());
}

RamDiskItem *RamDiskItem::GetParent() {
    return this->parent;
}

IDType RamDiskItem::GetID() {
    return this->id;
}

RamDiskItem *RamDiskItem::FindChildByName(lr::sstl::AString _Name) {
    auto i = this->children.Find(_Name);
    if ( i == this->children.End())return nullptr;
    else return i->second;
}

IDType RamDiskItem::AddChild(RamDiskItem *_Child) {
    Assert(this->type == DIR);
    _Child->parent = this;
    auto i = this->children.Insert(MakePair(_Child->name, _Child));
    return i.second == false ? -1 : _Child->id;
}

AString RamDiskItem::GetName() {
    return this->name;
}