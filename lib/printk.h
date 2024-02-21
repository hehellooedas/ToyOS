#ifndef __LIB_PRINTK_H
#define __LIB_PRINTK_H


#include <font.h>
#include <stdarg.h>
#include <screen.h>



#define ZEROPAD     1    // %0
#define SIGN        2    // %
#define PLUS        4    // %+
#define SPACE       8    // % 
#define LEFT        16   // %-
#define SPECIAL     32   // %#
#define SMALL       64   // %

/*  判断该字符是否在数字的范围  */
#define is_digit(x)     ((x) >= '0' && (x) <= '9')


/*  定义一些颜色  */
#define WHITE 	0x00ffffff		//白
#define BLACK 	0x00000000		//黑
#define RED	    0x00ff0000		//红
#define ORANGE	0x00ff8000		//橙
#define YELLOW	0x00ffff00		//黄
#define GREEN	0x0000ff00		//绿
#define BLUE	0x000000ff		//蓝
#define INDIGO	0x0000ffff		//靛
#define PURPLE	0x008000ff		//紫


/* 返回余数 */
#define do_div(n,base) ({ \
    int __res; \
    asm ("divq %%rcx":"=a"(n),"=d"(__res):"0"(n),"1"(0),"c"(base)); \
    __res; \
})



int skip_atoi(const char** s);
int vsprintf(char* buf,const char* fmt,va_list args);
int color_printk(unsigned int FRcolor,unsigned int BKcolor,const char* fmt,...);

#endif // !__LIB_PRINTK_H

