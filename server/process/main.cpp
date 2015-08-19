#include"stdio.h"
#include"SystemCalls.h"
#include"UserLog.h"

int main() {
    auto fp = fopen("/dev/hda", "rw");
    printf("fid: %d\n", fp);
    char data[512];
    auto size = fread(data,sizeof(data),1,fp);
    int *tmp = (int *) data;
    printf("read: %d", size);
    for ( int i = 0; i < sizeof(data) / sizeof(int); i++ ) {
        printf("%.8x ", tmp[i]);
        if ( i % 8 == 0 ) {
            printf("\n");
        }
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
