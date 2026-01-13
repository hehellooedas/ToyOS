#ifndef __LIB_FLAGS_H
#define __LIB_FLAGS_H

#include <printk.h>



/*
                             rflags程序状态字
 0 | ID | VIP | VIF | AC | (VM) | RF | 0 | IOPL | OF | DF | IF | TF |
 SF| ZF | 0 | AF | 0 | PF | 1 | CF |

状态标志位:CF PF AF ZF SF
方向标志位:DF 
系统标志位:TF IF IOPL NT RF VM AC VIF VIP ID
其中CF标志位可以使用stc clc cmc指令进行操作
*/


struct rflag_struct{
    unsigned int
        CF:1,           //进位标志位(未发生/发生)
        res_1:1,        //保留
        PF:1,           //奇偶标志位(奇数个1/偶数个1)
        res_2:1,        //保留
        AF:1,           //辅助标志位(用于BCD运算)
        res_3:1,        //保留
        ZF:1,           //zero零值标志位(结果为1/结果为0)
        SF:1,           //sign符号标志位(正数/负数)
        TF:1,           //跟踪标志位(开启单步调试)
        IF:1,           //中断使能标志位(开启可屏蔽中断)
        DF:1,           //方向标志位(正向/逆向)
        OF:1,           //溢出标志位(未发生溢出/发生了溢出)
        IOPL:2,         //当前IO操作的特权级
        NF:1,           //任务嵌套标志位(长模式不使用)
        res_4:1,        //保留
        RF:1,           //恢复标志位(允许调试异常)
        VM:1,           //虚拟8086模式标志位(不使用)
        AC:1,           //对齐检测标志位(数据对齐检测)
        VIF:1,          //虚拟中断标志位(虚拟的IF)
        VIP:1,          //虚拟中断挂起标志位
        ID:1,           //ID标志位(表明当前CPU是否支持CPUID指令)
        res_5:10;
    unsigned int res;
};




static __attribute__((always_inline))
struct rflag_struct get_rflags(void)
{
    unsigned long rflags;
    asm volatile (
        "pushfq     \n\t" //rflags入栈(f表示flags,q表示尺寸)
        "popq %0    \n\t" //取出保存在栈里的rflags
        "mfence     \n\t"
        :"=g"(rflags)
        :
        :"memory"
    );
    return *(struct rflag_struct*)&rflags;
}



static __attribute__((always_inline))
void print_current_rflags(void)
{
    struct rflag_struct rflags = get_rflags();
    color_printk(GREEN,BLACK ,"current rflags is " );

    if(rflags.CF) color_printk(GREEN,BLACK ,"CF " );
    if(rflags.PF) color_printk(GREEN,BLACK ,"PF " );
    if(rflags.AF) color_printk(GREEN,BLACK ,"AF " );
    if(rflags.ZF) color_printk(GREEN,BLACK ,"ZF " );
    if(rflags.SF) color_printk(GREEN,BLACK ,"SF " );
    if(rflags.TF) color_printk(GREEN,BLACK ,"TF " );
    if(rflags.IF) color_printk(GREEN,BLACK ,"IF " );
    if(rflags.DF) color_printk(GREEN,BLACK ,"DF " );
    if(rflags.OF) color_printk(GREEN,BLACK ,"OF " );
    if(rflags.RF) color_printk(GREEN,BLACK ,"RF " );
    if(rflags.AC) color_printk(GREEN,BLACK ,"AC " );
    if(rflags.VIF) color_printk(GREEN,BLACK ,"VIF " );
    if(rflags.VIP) color_printk(GREEN,BLACK ,"VIP " );
    if(rflags.ID) color_printk(GREEN,BLACK ,"ID " );


    color_printk(RED,BLACK,"\n");
}



static __attribute__((always_inline))
void clear_rflags(void)
{
    asm volatile (
        "pushq $0   \n\t"
        "popfq      \n\t"
        "mfence     \n\t"
    );
}

#endif // !__LIB_FLAGS_H

