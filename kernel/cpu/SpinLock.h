#pragma once
#include"Global.h"


class SpinLock
{
	private:
		char tmplock;
		char lock;
		int depth;
		int currCPU;
		bool basicMode;
	public:
		void Lock();
		void Unlock();
		bool Try();
		SpinLock(bool _BasicMode = false);
};

