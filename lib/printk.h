#ifndef __LIB_PRINTK_H
#define __LIB_PRINTK_H


#include <font.h>
#include <stdarg.h>



#define ZEROPAD     1    // %0
#define SIGN        2    // %
#define PLUS        4    // %+
#define SPACE       8    // % 
#define LEFT        16   // %-
#define SPECIAL     32   // %#
#define SMALL       64   // %

/*  判断该字符是否在数字的范围  */
#define is_digit(x)     ((x) >= '0' && (x) <= '9')



typedef unsigned int Color ;    //四字节表示一个像素


/*  定义一些颜色  */
#define WHITE 	0x00ffffff		//白色
#define BLACK 	0x00000000		//黑色
#define RED	    0x00ff0000		//红色
#define ORANGE	0x00ff8000		//橙色
#define YELLOW	0x00ffff00		//黄色
#define GREEN	0x0000ff00		//绿色
#define BLUE	0x000000ff		//蓝色
#define INDIGO	0x0000ffff		//靛色
#define PURPLE	0x008000ff		//紫色
#define PINK    0x00ffc0cb      //粉色
#define SNOW    0x00fffafa      //雪色
#define SKY     0x00f0ffff      //天蓝色





static __attribute__((always_inline))
Color gen_color(unsigned char Red,unsigned char Green,unsigned char Blue)
{
    return ((Red << 16) & 0xff0000) | ((Green << 8) & 0xff00) | (Blue & 0xff);
}




/* 返回余数 */
#define do_div(n,base) ({ \
    int __res; \
    asm ("divq %%rcx":"=a"(n),"=d"(__res):"0"(n),"1"(0),"c"(base)); \
    __res; \
})



int skip_atoi(const char** s);
int vsprintf(char* buf,const char* fmt,va_list args);
int color_printk(Color FRcolor,Color BKcolor,const char* fmt,...);

#endif // !__LIB_PRINTK_H

