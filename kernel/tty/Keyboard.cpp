//
// Created by ssj on 15-8-12.
//

#include"Global.h"
#include"tty/tty.h"

extern "C" void WakeUpQueue(struct tty_queue* q)
{
    q->wait.Wake();
}
