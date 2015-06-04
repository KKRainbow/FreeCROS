#pragma once
#include"Global.h"


class SpinLock
{
	private:
		char tmplock;
		char lock;
		int depth;
		int currCPU;
		static bool basicMode;
	public:
		void Lock();
		void Unlock();
		bool Try();
		static void SetBasicMode(bool _BasicMode)
		{
			SpinLock::basicMode = _BasicMode;
		}
		SpinLock();
};

