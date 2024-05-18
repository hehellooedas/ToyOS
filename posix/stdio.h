#ifndef __POSIX_STDIO_H
#define __POSIX_STDIO_H


#define SEEK_SET    0   //定位文件开头
#define SEEK_CUR    1   //定位当前位置
#define SEEK_END    2   //定位文件结束
#define SEEK_MAX    3


extern void putstring(char *string);
int sprintf(char* buf,const char* fmt,...);
int printf(const char* fmt,...);

#endif