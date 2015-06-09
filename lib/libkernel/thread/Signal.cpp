#include"Signal.h"
#include"Interrupts.h"
#include"SystemCalls.h"
static sigaction sigactions[32];
static void SignalLibHandler(int _Num,addr_t _Handler,InterruptParams _Params)
{
	sigactions[_Num].sa_handler(_Num,nullptr,nullptr);
	//恢复栈
	__asm__(
		"movl %0,%%eax\n\t"
		"movl %1,%%ebx\n\t"
		"movl %2,%%ecx\n\t"
		"movl %3,%%edx\n\t"
		"movl %4,%%edi\n\t"
		"movl %5,%%esi\n\t"
		"movl %6,%%ebp\n\t"
		"movl %7,%%esp\n\t"
		"jmp *%0\n\t"
		::"m"(_Params.eax),
		"m"(_Params.ebx),
		"m"(_Params.ecx),
		"m"(_Params.edx),
		"m"(_Params.edi),
		"m"(_Params.esi),
		"m"(_Params.ebp),
		"m"(_Params.esp),
		"m"(_Params.eip)
	);
}

sighandler_t Signal(int _Signum,sighandler_t _Sig,int _Flag)
{
	sigactions[_Signum].sa_handler = _Sig;
	sigactions[_Signum].sa_flags = _Flag;
	return (sighandler_t)
		SysCallSignal::Invoke((uint32_t)_Signum,(uint32_t)SignalLibHandler,(uint32_t)_Flag);
}