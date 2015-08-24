//
// Created by ssj on 15-8-24.
//
#pragma once

#include <stl/sstl/sstl_wstring.h>
#include"fat32.h"

#define IS_DIR(dir) (dir.attributes & 0x10)
#define IS_REG(dir) (dir.attributes & 0x20)

#define GET_CLUSTER_BEGIN(dir) ((uint32_t)dir.high_starting_cluster*256*256 + \
                                    dir.low_starting_cluster)

bool GetDirectoryEntry(lr::sstl::AString _Path, DirectoryEntry* _Root, Fat32Entry& _Res);
int GetContent(Fat32Entry* _Entry, off_t _Offset, size_t _Size, char* _Buf);
int Fat32Init(const char* _Dev);

