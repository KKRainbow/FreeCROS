#include <dirent.h>
#include"stdio.h"
#include"SystemCalls.h"
#include"UserLog.h"
#include "thread/Signal.h"

static FILE* fp = nullptr;
int sig_alarm()
{
    fp = (FILE*)444;
    return 1;
    printf("SIGNAL ALARM!!!\n");
    fp = fopen("/mnt/file","rb");
}

int main() {
    Signal(SIGALRM, (sighandler_t)sig_alarm,0);
    SysCallAlarm::Invoke(1000);
    while(fp == nullptr);
    log("Success to open file !!!\n");
    char tmp[200];
//    int read = fread(tmp, 100,1,fp);
//    log("%s", tmp);

    FILE* dp = fopen("/mnt/", "rb");
    SysCallMkdir::Invoke((uint32_t)"/mnt/shiti/a/b/c",(uint32_t)true);

    dirent dir[6];
    fread(dir, sizeof(dir[0]), 7, dp);
    int i = 0;
    while (i++ != 7)
    {
        printf("dir :   %s\n", dir[i-1].d_name);
    }
    for(;;);
    while(1)
    {
        char str[100];
        printf("please input a num\n");
        int n = scanf("%s",str);
        printf("your num is%s\n",str);
    }
    return 1;
}
