#pragma once

#ifndef __ASSEMBLER__

#include"stddef.h"

#include"Macros.h"

typedef long long int64_t;
typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned long long uint64_t;
typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef uint32_t reg64_t;
typedef uint32_t reg32_t;
typedef uint32_t reg16_t;
typedef uint32_t reg8_t;

typedef unsigned long addr_t;

typedef int pid_t;
typedef int irq_t;
typedef int off_t;

typedef unsigned long tcflag_t;
typedef unsigned char cc_t;
typedef unsigned int speed_t;

typedef int dev_t;

typedef int ino_t;
#endif