#include"cpu/SpinLock.h"
#include"cpu/CPUManager.h"
#define UNLOCKED_NUM 0xffffffff 
//改为检查是不是当前cpu
#define BASIC_LOCK(lock) \
	__asm__ volatile( \
			"1:\n\t" \
			"movb $1,%%al\n\t" \
			"lock xchgb %%al,(%0)\n\t" \
			"cmpb $0,%%al\n\t" \
			"jne 1b\n\t" \
			::"b"(&(lock)))
#define BASIC_UNLOCK(lock) \
		__asm__ volatile( \
				"movb $0,(%0)\n\t" \
				::"b"(&(lock)));
#define BASIC_TRY(res,lock) \
	do\
	{ \
		char a = 0;\
		__asm__ volatile(  \
				"2:\n\t"  \
				"movb $1,%%al\n\t"  \
				"lock xchg %%al,(%1)\n\t" \
				:"=a"((a)):"b"(&(lock))); \
		res = (a == 0);\
	}while(0)
void SpinLock::Lock()
{
	if(this->basicMode)
	{
		BASIC_LOCK(this->lock);
		return;
	}
	int id = CPUManager::Instance()->GetCurrentCPU()->GetID();
	BASIC_LOCK(tmplock);
	if(currCPU == id)
	{
		depth++;
		BASIC_UNLOCK(tmplock);
		return;
	}
	BASIC_UNLOCK(tmplock);
	BASIC_LOCK(this->lock);
	currCPU = id;
	depth++;
}

void SpinLock::Unlock()
{
	if(this->basicMode)
	{
		BASIC_UNLOCK(this->lock);
		return;
	}
	depth--;
	if(depth == 0)
	{
		currCPU = UNLOCKED_NUM;
		BASIC_UNLOCK(this->lock);
	}
}

bool SpinLock::Try()
{
	if(this->basicMode)
	{
		bool tmp;
		BASIC_TRY(tmp,this->lock);
		return tmp;
	}
	int id = CPUManager::Instance()->GetCurrentCPU()->GetID();
	bool res;

	BASIC_LOCK(tmplock);
	if(currCPU == id)
	{
		depth++;
		BASIC_UNLOCK(tmplock);
		return true;
	}
	BASIC_UNLOCK(tmplock);
	BASIC_TRY(res,this->lock);
	if(res)
	{
		currCPU = id;
		depth++;
		return true;
	}
	else
		return false;
}

SpinLock::SpinLock(bool _BasicMode)
{
	lock = 0;
	depth = 0;
	tmplock = 0;
	currCPU = UNLOCKED_NUM;
	basicMode = _BasicMode;
}
