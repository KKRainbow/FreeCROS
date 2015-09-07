//
// Created by ssj on 15-8-22.
//
#include "format.h"

lr::sstl::AString BaseName(lr::sstl::AString _Str) {
    auto iter = _Str.End();
    --iter;
    for ( ;(*iter == '/' && iter != _Str.Begin()); --iter);
    for ( ;(*iter != '/' && iter != _Str.Begin()); --iter);
    if(*iter == '/')
    {
        ++iter;
    }
    if (iter == _Str.End())
    {
        return ".";
    }
    return _Str.Sub(iter, _Str.End());
}

lr::sstl::AString DirName(lr::sstl::AString _Str) {
    auto iter = _Str.End();
    --iter;
    for ( ;(*iter == '/' && iter != _Str.Begin()); --iter);
    for ( ;(*iter != '/' && iter != _Str.Begin()); --iter);
    for ( ;(*iter == '/' && iter != _Str.Begin()); --iter);
    if(iter == _Str.Begin() && *iter != '/')
    {
        return ".";
    }
    return _Str.Sub(_Str.Begin(), ++iter);
}

