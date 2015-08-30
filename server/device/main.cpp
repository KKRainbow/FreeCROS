#include"SystemCalls.h"
#include"Threads.h"
#include"UserLog.h"

//int keyboard();
int fat32();

int main() {
    Message msg;
    int i = 0;
    char a[500];
    void *test = new char[500];
    printf("new: %x \n,", test);

//    int pid = SysCallCreateThread::Invoke((uint32_t) keyboard, 0, 0, 0);
    int pid2 = SysCallCreateThread::Invoke((uint32_t) fat32, 0, 0, 0);
    log("pid: %d\n", pid2);
    for(;;);

    int *tmp = (int *) 0x20000000;
    *tmp = 5;

    printf("start open\n");

    for ( ; ; ) {
        auto size = fread(a, sizeof(a),1,stdin);
        fwrite(a, strlen(a), 1,stdout);
        memset(a, 0, 500);
    }
    return 1;
}
