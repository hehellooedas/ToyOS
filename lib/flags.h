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

#define flag_CF     0x01      //进位标志位(未发生/发生)
#define flag_PF     0x04      //奇偶标志位(奇数个1/偶数个1)
#define flag_AF     0x10      //辅助标志位(用于BCD运算)
#define flag_ZF     0x40      //零值标志位(结果为1/结果为0)
#define flag_SF     0x80      //符号标志位(正数/负数)
#define flag_TF     0x100     //跟踪标志位(开启单步调试)
#define flag_IF     0x200     //中断使能标志位(开启可屏蔽中断)
#define flag_DF     0x400     //方向标志位(正向/逆向)
#define flag_OF     0x800     //溢出标志位(未发生溢出/发生了溢出)
#define flag_RF     0x8000    //恢复标志位(允许调试异常)
#define flag_AC     0x20000   //对齐检测标志位(数据对齐检测)
#define flag_VIF    0x40000   //虚拟中断标志位(虚拟的IF)
#define flag_VIP    0x80000   //虚拟中断挂起标志位
#define flag_ID     0x100000  //ID标志位(检测CPUID指令)


struct flag_struct{
    char flag_Name[14][4];
    unsigned int flag_Value[14];
};

const struct flag_struct flag_data = {
    .flag_Name = {"CF","PF","AF","ZF","SF","TF","IF","DF","OF","RF","AC","VIF","VIP","ID"},
    .flag_Value = {flag_CF,flag_PF,flag_AF,flag_ZF,flag_SF,flag_TF,flag_IF,flag_DF,flag_OF,flag_RF,flag_AC,flag_VIF,flag_VIF,flag_ID},
};

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
    for(int i=0;i<14;i++){
        if(rflags & flag_data.flag_Value[i]){
            color_printk(RED,BLACK,"%s = 1\t",flag_data.flag_Name[i]);
        }
    }
    color_printk(RED,BLACK,"\n");
}



static __attribute__((always_inline))
void clear_rflags(void)
{
    asm volatile (
        "pushfq     \n\t"
        "popq %rax  \n\t"
        "andq $0,%rax   \n\t"
        "pushq %rax \n\t"
        "popfq      \n\t"
        "mfence     \n\t"
    );
}

#endif // !__LIB_FLAGS_H

