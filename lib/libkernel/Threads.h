#pragma once


struct Message
{
	uint64_t timeStamp;
	pid_t source;
	pid_t destination;
	uint32_t content[8];
};


enum ThreadType{KERNEL,SERVER,USER};

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