#include <stdio.h>
#include <stdarg.h>
#include <printk.h>
#include <task.h>




int sprintf(char* buf,const char* fmt,...){
    int count = 0;

    va_list args;
    va_start(args,fmt );
    count = vsprintf(buf,fmt ,args );
    va_end(args);

    return count;
}



int printf(const char* fmt,...){
    char buf[1000];
    int count = 0;

    va_list args;
    va_start(args,fmt );
    count = vsprintf(buf,fmt ,args );
    va_end(args);

    putstring(buf);

    return count;
}