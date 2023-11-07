#ifndef __LIB_FLAGS_H
#define __LIB_FLAGS_H

#include "printk.h"
#include <bits/posix2_lim.h>



/*
                             rflags程序状态字
 0 | ID | VIP | VIF | AC | (VM) | RF | 0 | IOPL | OF | DF | IF | TF |
 SF| ZF | 0 | AF | 0 | PF | 1 | CF |
*/

#define flag_CF     0x01     //进位标志位
#define flag_PF     0x04     //奇偶标志位
#define flag_AF     0x10     //辅助标志位
#define flag_ZF     0x40     //零值标志位
#define flag_SF     0x80     //符号标志位
#define flag_TF     0x100    //跟踪标志位
#define flag_IF     0x200    //中断使能标志位
#define flag_DF     0x400    //方向标志位
#define flag_OF     0x800    //溢出标志位
#define flag_RF     0x8000   //恢复标志位
#define flag_AC     0x20000  //对齐检测标志位
#define flag_VIF    0x40000  //虚拟中断标志位
#define flag_VIP    0x80000  //虚拟中断挂起标志位
#define flag_ID     0x100000 //ID标志位



static __attribute__((always_inline))
unsigned long get_rflags(void)
{
    unsigned long rflags;
    asm volatile (
        "pushfq     \n\t"
        "popq %0    \n\t"
        "mfence     \n\t"
        :"=g"(rflags)
        :
        :"memory"
    );
    return rflags;
}


static __attribute__((always_inline))
void print_current_rflags(void)
{
    unsigned long rflags = get_rflags();
    color_printk(RED,BLACK,\
    "current rflags is %#b\n",rflags);
    if(rflags & flag_CF){
        color_printk(RED,BLACK,"CF = 1\t");
    }
    if(rflags & flag_PF){
        color_printk(RED,BLACK,"PF = 1\t");
    }
    if(rflags & flag_AF){
        color_printk(RED,BLACK,"AF = 1\t");
    }
    if(rflags & flag_ZF){
        color_printk(RED,BLACK,"ZF = 1\t");
    }
    if(rflags & flag_SF){
        color_printk(RED,BLACK,"SF = 1\t");
    }
    if(rflags & flag_TF){
        color_printk(RED,BLACK,"TF = 1\t");
    }
    if(rflags & flag_IF){
        color_printk(RED,BLACK,"IF = 1\t");
    }
    if(rflags & flag_DF){
        color_printk(RED,BLACK,"DF = 1\t");
    }
    if(rflags & flag_OF){
        color_printk(RED,BLACK,"OF = 1\t");
    }
    if(rflags & flag_RF){
        color_printk(RED,BLACK,"RF = 1\t");
    }
    if(rflags & flag_AC){
        color_printk(RED,BLACK,"AC = 1\t");
    }
    if(rflags & flag_VIF){
        color_printk(RED,BLACK,"VIF = 1\t");
    }
    if(rflags & flag_VIP){
        color_printk(RED,BLACK,"VIP = 1\t");
    }
    if(rflags & flag_ID){
        color_printk(RED,BLACK,"ID = 1\t");
    }
    color_printk(RED,BLACK,"\n");
}


#endif // !__LIB_FLAGS_H

