#include"stdio.h"
#include"SystemCalls.h"
#include"UserLog.h"

int main() {
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
