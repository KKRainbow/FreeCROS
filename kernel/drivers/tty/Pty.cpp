/*
 *  linux/kernel/chr_drv/pty.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 *	pty.c
 *
 * This module implements the pty functions
 *	void mpty_write(struct tty_struct * queue);
 *	void spty_write(struct tty_struct * queue);
 */

#include"Global.h"
#include <ctype.h>
#include <thread/ThreadManager.h>
#include <cpu/CPUManager.h>
#include "errno.h"
#include "thread/Signal.h"

#define ALRMMASK (1<<(SIGALRM-1))

#include "driver/tty/tty.h"
#include "driver/tty/termios.h"
#include "driver/tty/ttymacro.h"

static inline void pty_copy(struct tty_struct * from, struct tty_struct * to)
{
	char c;
	Thread* current = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();

	while (!from->stopped && !EMPTY(from->write_q)) {
		if (FULL(to->read_q)) {
			if (FULL(to->secondary))
				break;
			copy_to_cooked(to);
			continue;
		}
		GETCH(from->write_q,c);
		PUTCH(c,to->read_q);
		if (current->hasSignal())
			break;
	}
	copy_to_cooked(to);
	from->write_q->wait.Wake();
}

/*
 * This routine gets called when tty_write has put something into
 * the write_queue. It copies the input to the output-queue of it's
 * slave.
 */
void mpty_write(struct tty_struct * tty)
{
	int nr = tty - tty_table;

	if ((nr >> 6) != 2)
		LOG("bad mpty\n\r");
	else
		pty_copy(tty,tty+64);
}

void spty_write(struct tty_struct * tty)
{
	int nr = tty - tty_table;

	if ((nr >> 6) != 3)
		LOG("bad spty\n\r");
	else
		pty_copy(tty,tty-64);
}
