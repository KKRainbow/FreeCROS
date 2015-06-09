#include"Signal.h"
#include"Interrupts.h"
#include"SystemCalls.h"
static sigaction sigactions[32];
static void SignalLibHandler(int _Num,addr_t _Handler,InterruptParams _Params)
{
	sigactions[_Num].sa_handler(_Num,nullptr,nullptr);
	SysCallSignalRestore::Invoke();
}

sighandler_t Signal(int _Signum,sighandler_t _Sig,int _Flag)
{
	sigactions[_Signum].sa_handler = _Sig;
	sigactions[_Signum].sa_flags = _Flag;
	return (sighandler_t)
		SysCallSignal::Invoke((uint32_t)_Signum,(uint32_t)SignalLibHandler,(uint32_t)_Flag);
}
