//
// Created by ssj on 15-8-18.
//

#include <stdio.h>
#include "stdlib.h"

int main(int argc, char* argv[],char* env[]);

void init_stdio()
{
    for(auto& i : fp_table)
    {
        i.fd = -1;
    }
    stdin = fopen("/dev/tty","rw");
    stderr = fopen("/dev/tty","rw");
    stdout = fopen("/dev/tty","rw");
}

extern "C" int start(int argc, char* argv[],char* env[])
{
    init_stdio();
    int ret = main(0, nullptr, nullptr);
    exit(ret);
    return 0;
}