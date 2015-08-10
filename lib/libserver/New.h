//
// Created by ssj on 15-8-9.
//

#pragma once

#include "Server.h"
void* operator new(unsigned int size);
void* operator new(unsigned int size,void* p);
void* operator new[](unsigned int size);
void* operator new[](unsigned int size,void* p);
void operator delete(void* p) throw();
