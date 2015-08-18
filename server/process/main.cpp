#include"stdio.h"
#include"SystemCalls.h"
#include"UserLog.h"

int main() {
    auto fid = SysCallOpen::Invoke((uint32_t) "/dev/hda1");
    log("fid: %d\n", fid);
    char data[512];
    auto size = SysCallRead::Invoke(fid, (uint32_t) data, sizeof(data));
    int *tmp = (int *) data;
    log("read: %d", size);
    for ( int i = 0; i < sizeof(data) / sizeof(int); i++ ) {
        log("%x ", tmp[i]);
        if ( i % 10 == 0 ) {
            log("\n");
        }
    }
    for ( ; ; );
    return 1;
}
