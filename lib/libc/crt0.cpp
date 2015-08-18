//
// Created by ssj on 15-8-18.
//

#include "crt0.h"

int main(int argc, char* argv[],char* env[]);

extern "C" int start()
{
    return main(0, nullptr, nullptr);
}