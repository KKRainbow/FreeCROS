//
// Created by ssj on 15-8-18.
//

#include "stdlib.h"
#include "../libkernel/SystemCalls.h"

extern "C" void exit(int status)
{
    SysCallExit::Invoke(status);
}
