#include"stdio.h"
#include"SystemCalls.h"
#include"UserLog.h"
int main()
{
	char a[500];
	int res = 10;
	int n  =snprintf(a,sizeof(a),"fdsakfldajI/O: %d \n",res);
	SysCallLog::Invoke((uint32_t)a,n,0,0);	
	
	log("fijdsoaifjsdoaijfosdaj\n");
	for(;;);
	for(;;)
	{
		//char a[] = "Fdsafdsafsda\n";
		//SysCallLog::Invoke((uint32_t)a,sizeof(a),0,0);	
		//for(int i = 0;i<0x10000000;i++);
		//SysCallReceiveAll::Invoke(0,0,0,0);
		int res = 0;
		res = Inb(0x21);
		int n  =snprintf(a,sizeof(a),"I/O: %d \n",res);
		SysCallLog::Invoke((uint32_t)a,n,0,0);	
	}
	return 1;
}
