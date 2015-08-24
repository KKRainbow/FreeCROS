//
// Created by ssj on 15-8-22.
//
#pragma once

#include "Type.h"
#include "thread/Message.h"

struct FsMsg
{
    enum{
        O_PATH,
        O_PATH_SIZE,
        O_MODE,
        O_MODE_SIZE,
    };
    enum{
        R_FD,
        R_POS,
        R_BUF,
        R_SIZE,
    };
    enum{
        W_FD,
        W_POS,
        W_BUF,
        W_SIZE,
    };
    //mkdir
    enum{
        M_PATH,
        M_PATH_SIZE,
        M_MODE,
        M_MODE_SIZE,
        M_RECURSIVE,
    };
};
