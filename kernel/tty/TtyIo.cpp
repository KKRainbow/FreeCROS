/*
 *  linux/kernel/tty_io.c
 *
 *  (C) 1991  Linus Torvalds
 */

/*
 * 'tty_io.c' gives an orthogonal feeling to tty's, be they consoles
 * or rs-channels. It also implements echoing, cooked mode etc.
 *
 * Kill-line thanks to John T Kohl, who also corrected VMIN = VTIME = 0.
 */

#include"Global.h"
#include <ctype.h>
#include <thread/ThreadManager.h>
#include <cpu/CPUManager.h>
#include "errno.h"
#include "thread/Signal.h"

#define ALRMMASK (1<<(SIGALRM-1))

#include "tty/tty.h"
#include "tty/termios.h"
#include "tty/ttymacro.h"

#define _L_FLAG(tty,f)	((tty)->termios.c_lflag & f)
#define _I_FLAG(tty,f)	((tty)->termios.c_iflag & f)
#define _O_FLAG(tty,f)	((tty)->termios.c_oflag & f)

#define L_CANON(tty)	_L_FLAG((tty),ICANON)
#define L_ISIG(tty)	_L_FLAG((tty),ISIG)
#define L_ECHO(tty)	_L_FLAG((tty),ECHO)
#define L_ECHOE(tty)	_L_FLAG((tty),ECHOE)
#define L_ECHOK(tty)	_L_FLAG((tty),ECHOK)
#define L_ECHOCTL(tty)	_L_FLAG((tty),ECHOCTL)
#define L_ECHOKE(tty)	_L_FLAG((tty),ECHOKE)
#define L_TOSTOP(tty)	_L_FLAG((tty),TOSTOP)

#define I_UCLC(tty)	_I_FLAG((tty),IUCLC)
#define I_NLCR(tty)	_I_FLAG((tty),INLCR)
#define I_CRNL(tty)	_I_FLAG((tty),ICRNL)
#define I_NOCR(tty)	_I_FLAG((tty),IGNCR)
#define I_IXON(tty)	_I_FLAG((tty),IXON)

#define O_POST(tty)	_O_FLAG((tty),OPOST)
#define O_NLCR(tty)	_O_FLAG((tty),ONLCR)
#define O_CRNL(tty)	_O_FLAG((tty),OCRNL)
#define O_NLRET(tty)	_O_FLAG((tty),ONLRET)
#define O_LCUC(tty)	_O_FLAG((tty),OLCUC)

#define C_SPEED(tty)	((tty)->termios.c_cflag & CBAUD)
#define C_HUP(tty)	(C_SPEED((tty)) == B0)

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define QUEUES	(3*(MAX_CONSOLES+NR_SERIALS+2*NR_PTYS))
static struct tty_queue tty_queues[QUEUES];
struct tty_struct tty_table[256];
static SpinLock lock;

#define con_queues tty_queues
#define rs_queues ((3*MAX_CONSOLES) + tty_queues)
#define mpty_queues ((3*(MAX_CONSOLES+NR_SERIALS)) + tty_queues)
#define spty_queues ((3*(MAX_CONSOLES+NR_SERIALS+NR_PTYS)) + tty_queues)

#define con_table tty_table
#define rs_table (64+tty_table)
#define mpty_table (128+tty_table)
#define spty_table (192+tty_table)

int fg_console = 0;

/*
 * these are the tables used by the machine code handlers.
 * you can implement virtual consoles.
 */
struct tty_queue * table_list[]={
	con_queues + 0, con_queues + 1,
	rs_queues + 0, rs_queues + 1,
	rs_queues + 3, rs_queues + 4
	};

extern C void change_console(unsigned int new_console)
{
	if (new_console == fg_console || new_console >= NR_CONSOLES)
		return;
	fg_console = new_console;
	table_list[0] = con_queues + 0 + fg_console*3;
	table_list[1] = con_queues + 1 + fg_console*3;
	update_screen();
}

static void sleep_if_empty(struct tty_queue * queue)
{
	Interrupt::Cli();
	Thread* current = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	while (!current->hasSignal() && EMPTY(queue))
		queue->wait.Wait();
	Interrupt::Sti();
}

static void sleep_if_full(struct tty_queue * queue)
{
	if (!FULL(queue))
		return;
	Thread* current = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	Interrupt::Cli();
	while (!current->hasSignal() && LEFT(queue)<128)
		queue->wait.Wait();
	Interrupt::Sti();
}

void wait_for_keypress(void)
{
	sleep_if_empty(tty_table[fg_console].secondary);
}

void copy_to_cooked(struct tty_struct * tty)
{
	signed char c;

	if (!(tty->read_q || tty->write_q || tty->secondary)) {
		LOG("copy_to_cooked: missing queues\n\r");
		return;
	}
	while (1) {
		if (EMPTY(tty->read_q))
			break;
		if (FULL(tty->secondary))
			break;
		GETCH(tty->read_q,c);
		if (c==13) {
			if (I_CRNL(tty))
				c=10;
			else if (I_NOCR(tty))
				continue;
		} else if (c==10 && I_NLCR(tty))
			c=13;
		if (I_UCLC(tty))
			c=tolower(c);
		if (L_CANON(tty)) {
			if ((KILL_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==KILL_CHAR(tty))) {
				/* deal with killing the input line */
				while(!(EMPTY(tty->secondary) ||
				        (c=LAST(tty->secondary))==10 ||
				        ((EOF_CHAR(tty) != _POSIX_VDISABLE) &&
					 (c==EOF_CHAR(tty))))) {
					if (L_ECHO(tty)) {
						if (c<32)
							PUTCH(127,tty->write_q);
						PUTCH(127,tty->write_q);
						tty->write(tty);
					}
					DEC(tty->secondary->head);
				}
				continue;
			}
			if ((ERASE_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==ERASE_CHAR(tty))) {
				if (EMPTY(tty->secondary) ||
				   (c=LAST(tty->secondary))==10 ||
				   ((EOF_CHAR(tty) != _POSIX_VDISABLE) &&
				    (c==EOF_CHAR(tty))))
					continue;
				if (L_ECHO(tty)) {
					if (c<32)
						PUTCH(127,tty->write_q);
					PUTCH(127,tty->write_q);
					tty->write(tty);
				}
				DEC(tty->secondary->head);
				continue;
			}
		}
		if (I_IXON(tty)) {
			if ((STOP_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==STOP_CHAR(tty))) {
				tty->stopped=1;
				tty->write(tty);
				continue;
			}
			if ((START_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==START_CHAR(tty))) {
				tty->stopped=0;
				tty->write(tty);
				continue;
			}
		}
		if (L_ISIG(tty)) {
			if ((INTR_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==INTR_CHAR(tty))) {
                ThreadManager::Instance()->KillProcessGroup(tty->pgrp, SIGINT, 1);
				continue;
			}
			if ((QUIT_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==QUIT_CHAR(tty))) {
				ThreadManager::Instance()->KillProcessGroup(tty->pgrp, SIGQUIT, 1);
				continue;
			}
			if ((SUSPEND_CHAR(tty) != _POSIX_VDISABLE) &&
			    (c==SUSPEND_CHAR(tty))) {
				if (!ThreadManager::Instance()->IsOrphaned(tty->pgrp))
					ThreadManager::Instance()->KillProcessGroup(tty->pgrp, SIGTSTP, 1);
				continue;
			}
		}
		if (c==10 || (EOF_CHAR(tty) != _POSIX_VDISABLE &&
			      c==EOF_CHAR(tty)))
			tty->secondary->data++;
        PUTCH(c,tty->secondary);
		if (L_ECHO(tty)) {
			if (c==10) {
				PUTCH(10,tty->write_q);
				PUTCH(13,tty->write_q);
			} else if (c<32) {
				if (L_ECHOCTL(tty)) {
					PUTCH('^',tty->write_q);
					PUTCH(c+64,tty->write_q);
				}
			} else
				PUTCH(c,tty->write_q);
			tty->write(tty);
		}
	}
	tty->secondary->wait.Wake();
}

/*
 * Called when we need to send a SIGTTIN or SIGTTOU to our process
 * group
 * 
 * We only request that a system call be restarted if there was if the 
 * default signal handler is being used.  The reason for this is that if
 * a job is catching SIGTTIN or SIGTTOU, the signal handler may not want 
 * the system call to be restarted blindly.  If there is no way to reset the
 * terminal pgrp back to the current pgrp (perhaps because the controlling
 * tty has been released on logout), we don't want to be in an infinite loop
 * while restarting the system call, and have it always generate a SIGTTIN
 * or SIGTTOU.  The default signal handler will cause the process to stop
 * thus avoiding the infinite loop problem.  Presumably the job-control
 * cognizant parent will fix things up before continuging its child process.
 */
int tty_signal(int sig, struct tty_struct *tty)
{
	Thread* current = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	if (ThreadManager::Instance()->IsOrphaned(current->GetPgrp()))
		return -EIO;		/* don't stop an orphaned pgrp */
	ThreadManager::Instance()->KillProcessGroup(current->GetPgrp(),sig,1);
//	if ((current->blocked & (1<<(sig-1))) ||
//	    ((int) current->sigaction[sig-1].sa_handler == 1))
//		return -EIO;		/* Our signal will be ignored */
//	else if (current->sigaction[sig-1].sa_handler)
//		return -EINTR;		/* We _will_ be interrupted :-) */
//	else
//		return -ERESTARTSYS;	/* We _will_ be interrupted :-) */
//					/* (but restart after we continue) */
}

int tty_read(unsigned channel, char * buf, int nr)
{
	struct tty_struct * tty;
	struct tty_struct * other_tty = nullptr;
	char c, * b=buf;
	int minimum,time;

	Thread* current = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();
	if (channel > 255)
		return -EIO;
	tty = TTY_TABLE(channel);
	if (!(tty->write_q || tty->read_q || tty->secondary))
		return -EIO;
	if ((current->GetTty() == channel) && (tty->pgrp != current->GetPgrp()))
		return(tty_signal(SIGTTIN, tty));
	if (channel & 0x80)
		other_tty = tty_table + (channel ^ 0x40);
	time = 10L*tty->termios.c_cc[VTIME];
	minimum = tty->termios.c_cc[VMIN];
	if (L_CANON(tty)) {
		minimum = nr;
		current->ResetAlarm();
		time = 0;
	} else if (minimum)
		current->ResetAlarm();
	else {
		minimum = nr;
		if (time)
			current->Alarm((uint32_t)time);
		time = 0;
	}
	if (minimum>nr)
		minimum = nr;
	while (nr>0) {
		if (other_tty)
			other_tty->write(other_tty);
        Interrupt::Cli();
		if (EMPTY(tty->secondary) || (L_CANON(tty) &&
		    !FULL(tty->read_q) && !tty->secondary->data)) {
			if (current->GetAlarm()!=0 ||
			  current->hasSignal()) {
                Interrupt::Sti();
				break;
			}
			if (IS_A_PTY_SLAVE(channel) && C_HUP(other_tty))
				break;
			tty->secondary->wait.Wait();
            Interrupt::Sti();
			continue;
		}
        Interrupt::Sti();
		do {
			GETCH(tty->secondary,c);
			if ((EOF_CHAR(tty) != _POSIX_VDISABLE &&
			     c==EOF_CHAR(tty)) || c==10)
				tty->secondary->data--;
			if ((EOF_CHAR(tty) != _POSIX_VDISABLE &&
			     c==EOF_CHAR(tty)) && L_CANON(tty))
				break;
			else {
                *b++ = c;
//                AddressSpaceManager::
//                Instance()->
//                        CopyDataFromAnotherSpace(*current->GetAddressSpace() ,
//                                                 b++,
//                                                 *AddressSpaceManager::Instance()->GetKernelAddressSpace(),
//                                                 &c,1);
				if (!--nr)
					break;
			}
			if (c==10 && L_CANON(tty))
				break;
		} while (nr>0 && !EMPTY(tty->secondary));
		tty->read_q->wait.Wake();
		if (time)
			current->Alarm((uint32_t) time);
		if (L_CANON(tty) || b-buf >= minimum)
			break;
	}
	current->ResetAlarm();
	if (current->hasSignal() && !(b-buf))
		return -ERESTARTSYS;
	return (int)(b-buf);
}

int tty_write(unsigned channel, char * buf, int nr)
{
	static int cr_flag=0;
	struct tty_struct * tty;
	char c, *b=buf;
    Thread* current = CPUManager::Instance()->GetCurrentCPU()->GetCurrThreadRunning();

	if (channel > 255)
		return -EIO;
	tty = TTY_TABLE(channel);
	if (!(tty->write_q || tty->read_q || tty->secondary))
		return -EIO;
	if (L_TOSTOP(tty) && 
	    (current->GetTty() == channel) && (tty->pgrp != current->GetPgrp()))
		return(tty_signal(SIGTTOU, tty));
	while (nr>0) {
		sleep_if_full(tty->write_q);
		if (current->hasSignal())
			break;
		while (nr>0 && !FULL(tty->write_q)) {
            AddressSpaceManager::
            Instance()->
                    CopyDataFromAnotherSpace(*AddressSpaceManager::Instance()->GetKernelAddressSpace(),
                                             &c,
                                             *current->GetAddressSpace() ,
                                             b,
                                             1);
			if (O_POST(tty)) {
				if (c=='\r' && O_CRNL(tty))
					c='\n';
				else if (c=='\n' && O_NLRET(tty))
					c='\r';
				if (c=='\n' && !cr_flag && O_NLCR(tty)) {
					cr_flag = 1;
					PUTCH(13,tty->write_q);
					continue;
				}
				if (O_LCUC(tty))
					c=toupper(c);
			}
			b++; nr--;
			cr_flag = 0;
			PUTCH(c,tty->write_q);
		}
		tty->write(tty);
		if (nr>0)
        {
            CPUManager::Instance()->GetCurrentCPU()->ExhaustCurrThread();
            CPUManager::Instance()->GetCurrentCPU()->Run();
        }
	}
	return (int)(b-buf);
}

/*
 * Jeh, sometimes I really like the 386.
 * This routine is called from an interrupt,
 * and there should be absolutely no problem
 * with sleeping even in an interrupt (I hope).
 * Of course, if somebody proves me wrong, I'll
 * hate intel for all time :-). We'll have to
 * be careful and see to reinstating the interrupt
 * chips before calling this, though.
 *
 * I don't think we sleep here under normal circumstances
 * anyway, which is good, as the task sleeping might be
 * totally innocent.
 */
extern C void do_tty_interrupt(int tty)
{
	copy_to_cooked(TTY_TABLE(tty));
}

void chr_dev_init(void)
{
}

void tty_init(void)
{
	int i;

	for (i=0 ; i < QUEUES ; i++)
		tty_queues[i] = (struct tty_queue) {0,0,0,0,'\0'};
	rs_queues[0] = (struct tty_queue) {0x3f8,0,0,0,'\0'};
	rs_queues[1] = (struct tty_queue) {0x3f8,0,0,0,'\0'};
	rs_queues[3] = (struct tty_queue) {0x2f8,0,0,0,'\0'};
	rs_queues[4] = (struct tty_queue) {0x2f8,0,0,0,'\0'};
	for (i=0 ; i<256 ; i++) {
		tty_table[i] =  (struct tty_struct) {
		 	{0, 0, 0, 0, 0, },
			0, 0, 0, nullptr, nullptr, nullptr, nullptr
		};
        strcpy((char*)tty_table[i].termios.c_cc,INIT_C_CC);
	}
	con_init();
	for (i = 0 ; i<NR_CONSOLES ; i++) {
		con_table[i] = (struct tty_struct) {
		 	{ICRNL,		/* change incoming CR to NL */
			OPOST|ONLCR,	/* change outgoing NL to CRNL */
			0,
			IXON | ISIG | ICANON | ECHO | ECHOCTL | ECHOKE,
			0,		/* console termio */
			},
			0,			/* initial pgrp */
			0,			/* initial session */
			0,			/* initial stopped */
			con_write,
			con_queues+0+i*3,con_queues+1+i*3,con_queues+2+i*3
		};
        strcpy((char*)con_table[i].termios.c_cc,INIT_C_CC);
	}
//	for (i = 0 ; i<NR_SERIALS ; i++) {
//		rs_table[i] = (struct tty_struct) {
//			{0, /* no translation */
//			0,  /* no translation */
//			B2400 | CS8,
//			0,
//			0,
//			},
//			0,
//			0,
//			0,
//			rs_write,
//			rs_queues+0+i*3,rs_queues+1+i*3,rs_queues+2+i*3
//		};
//        strcpy((char*)rs_table[i].termios.c_cc,INIT_C_CC);
//	}
	for (i = 0 ; i<NR_PTYS ; i++) {
		mpty_table[i] = (struct tty_struct) {
			{0, /* no translation */
			0,  /* no translation */
			B9600 | CS8,
			0,
			0,
			},
			0,
			0,
			0,
			mpty_write,
			mpty_queues+0+i*3,mpty_queues+1+i*3,mpty_queues+2+i*3
		};
        strcpy((char*)mpty_table[i].termios.c_cc,INIT_C_CC);
		spty_table[i] = (struct tty_struct) {
			{0, /* no translation */
			0,  /* no translation */
			B9600 | CS8,
			IXON | ISIG | ICANON,
			0,
			},
			0,
			0,
			0,
			spty_write,
			spty_queues+0+i*3,spty_queues+1+i*3,spty_queues+2+i*3
		};
        strcpy((char*)spty_table[i].termios.c_cc,INIT_C_CC);
	}
//	rs_init();
	LOG("%d virtual consoles\n\r",NR_CONSOLES);
	LOG("%d pty's\n\r",NR_PTYS);
}
