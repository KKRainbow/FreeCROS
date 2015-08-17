#include"SystemCalls.h"
#include"Threads.h"
#include"UserLog.h"

void keyboard();

int main() {
    Message msg;
    int i = 0;
    char a[500];
    void *test = new char[500];
    log("new: %x \n,", test);

    int pid = SysCallCreateThread::Invoke((uint32_t) keyboard, 0, 0, 0);
    log("pid: %d\n", pid);

    int *tmp = (int *) 0x20000000;
    *tmp = 5;

    log("start open\n");
    auto fid = -1;
    while ( 1 ) {
        fid = SysCallOpen::Invoke((uint32_t) "/dev/tty", 0, 0, 0);
        if ( fid >= 0 )break;
    }
    log("Get dev_t : %d\n", fid);

    for ( ; ; ) {
        auto size = SysCallRead::Invoke(fid, (uint32_t) a, 500, 0);
        SysCallWrite::Invoke(fid, (uint32_t) a, strlen(a));
        memset(a, 0, 500);
    }
    return 1;
}
