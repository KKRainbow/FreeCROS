#include"SystemCalls.h"
#include"Threads.h"
#include"stdio.h"
#include"UserLog.h"
#include"Device.h"
void keyboard();
int main()
{
	Message msg;
	int i = 0;
	char a[500];
	
	int pid = SysCallCreateThread::Invoke((uint32_t)keyboard,0,0,0);
	log("pid: %d\n",pid);
	
	log("start open\n");
	auto fid = -1;
	while(1)
	{
		fid = SysCallOpen::Invoke((uint32_t)"/dev/keyboard",0,0,0);
		if(fid >= 0)break;
	}
	log("Get dev_t : %d\n",fid);
	
	for(;;)
	{
		auto size = SysCallRead::Invoke(fid,(uint32_t)a,500,0);
		log(a);
	}
	return 1;
}
