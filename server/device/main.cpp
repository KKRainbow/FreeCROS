#include"SystemCalls.h"
#include"Threads.h"
#include"UserLog.h"

void keyboard();

int main() {
    Message msg;
    int i = 0;
    char a[500];
    void *test = new char[500];
    printf("new: %x \n,", test);

    int pid = SysCallCreateThread::Invoke((uint32_t) keyboard, 0, 0, 0);
    printf("pid: %d\n", pid);

    int *tmp = (int *) 0x20000000;
    *tmp = 5;

    printf("start open\n");
    return 1;

    for ( ; ; ) {
        auto size = fread(a, sizeof(a),1,stdin);
        fwrite(a, strlen(a), 1,stdout);
        memset(a, 0, 500);
    }
    return 1;
}
