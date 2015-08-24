#pragma once
#include "Type.h"
#include"thread/Message.h"
#include"thread/Signal.h"


enum ThreadType{KERNEL,SERVER,USER};

class RamDiskItem;
struct File {
public:
    unsigned short f_flags = 0;
    RamDiskItem* f_item = nullptr;
    bool close_on_exec = 0;
    off_t f_pos = 0;
    File* f_redirct_ptr = nullptr;
    ino_t f_inner;
};