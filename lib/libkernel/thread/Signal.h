#pragma once
#include"Global.h"
typedef uint32_t sigset_t;

struct sigval{
	int sival_int;
	void* sival_ptr;
};

struct siginfo{
	int si_signo;
	int si_errno;
	int si_code;
	sigval si_value;
};

typedef void (*sighandler_t)(int,siginfo*,void*);
struct sigaction
{
	union{
		sighandler_t sa_handler;
		void (*lib_sa_handler)();
	};
	sigset_t sa_mask;
	unsigned long sa_flags;
};
sighandler_t Signal(int _Signum,sighandler_t _Sig,int _Flag);


typedef unsigned int sigset_t;		/* 32 bits */

#define _NSIG             32
#define NSIG		_NSIG

#define SIGHUP		 1
#define SIGINT		 2
#define SIGQUIT		 3
#define SIGILL		 4
#define SIGTRAP		 5
#define SIGABRT		 6
#define SIGIOT		 6
#define SIGUNUSED	 7
#define SIGFPE		 8
#define SIGKILL		 9
#define SIGUSR1		10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGTERM		15
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22

/* Ok, I haven't implemented sigactions, but trying to keep headers POSIX */
#define SA_NOCLDSTOP	1
#define SA_INTERRUPT	0x20000000
#define SA_NOMASK	0x40000000
#define SA_ONESHOT	0x80000000

#define SIG_BLOCK          0	/* for blocking signals */
#define SIG_UNBLOCK        1	/* for unblocking signals */
#define SIG_SETMASK        2	/* for setting the signal mask */

#define SIG_DFL		((void (*)(int))0)	/* default signal handling */
#define SIG_IGN		((void (*)(int))1)	/* ignore signal */
#define SIG_ERR		((void (*)(int))-1)	/* error return from signal */

#ifdef notdef
#define sigemptyset(mask) ((*(mask) = 0), 1)
#define sigfillset(mask) ((*(mask) = ~0), 1)
#endif

