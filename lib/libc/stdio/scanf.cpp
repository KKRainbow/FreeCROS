//
// Created by ssj on 15-8-18.
//

#include <ctype.h>
#include <stdarg.h>
#include <Type.h>
#include "stdio.h"

//检测所输入的字符串str是否全为空格,若是,则重新输入,这也是scanf是I/O阻塞型的原因.
static uint8_t scan_skip(char *str, uint8_t i)
{
    FILE *fp = stdin;
    loop:
    //isspace检测所输入的字符是否为空格,返回空格数.
    while(isspace(str[i]))
    {
        i++;
    }
    //如果一直到字符串结束都是空格,则重新输入
    if (str[i] == 0)
    {
        i = 0;
        fread(str,sizeof(str),1,fp);
        goto loop;
    }

    //返回第一个非空格字符的下标
    return i;
}

//如果以%d/%u格式读取数,则对其字符串str中的数字字符转成base进制的数.
//参数base 是进行进制转换的基数,*nb用来保存所转换后的结果.(eg: base= 8则为八
//进制数)
static uint8_t scan_int(char *str,uint8_t i, uint8_t base,int *nb)
{
    uint32_t n = 0;
    int j,sign = 0;

    //检测此十进制数的正,负性
    switch(str[i])
    {
        case '-':
            sign = 1;
        case '+':
            i++;
            break;
    }

    while(1)
    {
        if (isdigit(str[i]))    //判断是否为数字'0' ~ '9'
        {
            j = str[i] - '0';
        }else if (isalpha(str[i])) //判断是否为字母
        {
            j = toupper(str[i]) - 'A' + 10; //用来计算16进制数
        }
        else
            break;
        if (j >= base)
            break;

        n = base * n + j;   //将字符转换成base进制的数
        i++;
    }
    *nb = (sign == 0 ? n: -n);  //正负数的判读

    //返回当前已读到的最后一个字符的下标
    return i;
}

//以长整型格式输入,则对其字符串str中的数字字符转成base进制的数,
//i为当前所要读取的字符的下标,*nb用来保存所转换后的结果.
static uint8_t scan_long(char *str,uint8_t i,uint8_t base,int *nb)
{
    uint32_t n = 0;
    int j,sign = 0;

    //对所读取的正负性判断
    switch (str[i])
    {
        case '-':
            sign = 1;
        case '+':
            i++;
            break;
    }


    while (1)
    {
        if (isdigit(str[i])) //判断是否为数字
        {
            j = str[i] - '0';
        }else if (isalpha(str[i])) //判断是否为字母
        {
            j = toupper(str[i]) - 'A' + 10;  //用来计算16进制的数
        }else
            break;
        if (j >= base)
            break;

        n = n * base + j;
        i++;
    }
    *nb = (sign == 0? n: -n); //对正负数的判读

    //返回当前已读到的最后一个字符的下标
    return i;

}


static uint8_t scan_float(char *str,uint8_t i,uint8_t base,float *nb)
{
    float n = 0.0, j, m = 0;
    uint8_t sign = 0;
    int flag = 0 , k =0;


    switch (str[i])
    {
        case '-':
            sign = 1;
        case '+':
            i++;
            break;
    }

    while (1)
    {
        //计算整数部分
        if (isdigit(str[i]) && flag == 0)
        {
            j = str[i] - '0';
            n = base * n + j;
        } else if (str[i] == '.')
        {
            flag = 1;   //作为标记符
            i++;
            continue;
        }else if (isdigit(str[i]) && flag == 1)     //计算小数部分
        {
            j = str[i] - '0';
            m = base * m + j;

            k++; //计算小数点位数
            if (k > 5)
                break;
        }else
            break;

        i++;
    }

    //将小数点六位后的数字忽略掉
    while(1)
    {
        if (isdigit(str[i]))
        {
            i++;
            continue;
        }else
            break;
    }
    switch (k)
    {
        case 1:
            n = n + m * 1e-1;
            break;
        case 2:
            n = n + m * 1e-2;
            break;
        case 3:
            n = n + m * 1e-3;
            break;
        case 4:
            n = n + m * 1e-4;
            break;
        case 5:
            n = n + m * 1e-5;
            break;
        default:
            n = n + m * 1e-6;
            break;
    }

    *nb = (sign == 0? n : -n);

    return i;

}


extern "C" int scanf(const char *format, ...)
{
    //定义一个指向形参列表的指针pArg
    va_list pArg;
    char str[512];
    uint8_t i = 0;
    int8_t nb = 0; //记录参数个数

    FILE *fp = stdin;

    int count = fread(str,sizeof(str),1,fp);

    va_start(pArg,format);//让pArg指向函数参数列表中的最后一个明确的参数,这里就是format参数.


    for(;*format != 0; format++)
    {

        if (isspace(*format))
            continue;
        //找到所输入的字符串中第一个非空格字符的下标,或I/O阻塞,从新输入
        i = scan_skip(str,i);


        if (*format == '%')
        {
            switch(*(++format))
            {
                case 'c':
                    /* char */
                    *va_arg(pArg,char *) = str[i++]; //给pArg所指向的地址空间单元赋值
                    break;
                case 'd':
                    /* decimal int */
                case 'u':
                    /* unsigned int */
                    i = scan_int(str,i,10,va_arg(pArg,int *));  //va_arg()用来返回pArg所指向的地址单元中的值
                    break;
                case 'f':
                    i = scan_float(str,i,10,va_arg(pArg,float *));
                    break;
                case 'o':
                    i = scan_int(str,i,8,va_arg(pArg,int *));
                    break;
                case 'x':
                    i = scan_int(str,i,16,va_arg(pArg,int *));
                    break;
                case 's':
                {
                    int8_t j = 0;
                    char *d = va_arg(pArg,char *);
                    while ((d[j++] = str[i++]) != 0)
                    {
                        if(--count == 0)break;
                        if(d[j-1] == '\n'){
                            d[j-1] = '\0';
                            break;
                        }
                    }
                }
                    break;
                    /* long */
                case 'l':
                    switch (*(++format))
                    {
                        case 'd':
                            /*decimal long */
                        case 'u':
                            i = scan_long(str,i,10,va_arg(pArg,int *));
                            break;
                        case 'o':
                            i = scan_long(str,i,8,va_arg(pArg,int *));
                            break;
                        case 'x':
                            i = scan_long(str,i,16,va_arg(pArg,int *));
                            break;
                    }
                    break;
                default:
                    if (str[i] != *format)
                        return -1;
                    break;
            }
            nb++;
        }else if (str[i] != *format)
            return -1;
    }

//使pArg不再指向堆栈,结束对形参的取值
    va_end(pArg);

//返回参数个数
    return nb;
}
