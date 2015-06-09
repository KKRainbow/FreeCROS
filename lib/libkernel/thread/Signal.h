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


static const int SIGALARM = 0;
static const int SIGINT = 1;