#include"stdio.h"
#include"SystemCalls.h"
#include"UserLog.h"
int main()
{
	auto fid = SysCallOpen::Invoke((uint32_t)"/dev/hda");
	char data[30];
    SysCallSeek::Invoke(fid,0xc00,SEEK_SET);
	auto size = SysCallRead::Invoke(fid,(uint32_t)data,sizeof(data));
    for( int i = 0;i<sizeof(data);i++)
        if(data[i] == '\0')data[i] = ' ';
    data[29] = '\0';
    log(data);
	for(;;);
	return 1;
}
