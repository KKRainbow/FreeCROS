//
// Created by ssj on 15-8-12.
//

#include "Drivers.h"
#include "tty/Tty.h"
#include "hd/HardDrive.h"
void InitDrivers()
{
    InitTty();
    InitHd();
}