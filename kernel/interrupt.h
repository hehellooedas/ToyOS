#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include <linkage.h>
#include <printk.h>
#include <ptrace.h>
#include <lib.h>
#include <io.h>


void interrupt_init(void);


static __attribute__((always_inline))
void sti(void)
{
    asm volatile (
        "sti    \n\t"
        :
        :
        :"memory"
    );
}


static __attribute__((always_inline))
void cli(void)
{
    asm volatile (
        "cli   \n\t"
        :
        :
        :"memory"
    );
}


#define NR_IRQS 24



typedef struct hw_int_type{
    void (*enable)(unsigned long irq);      //使能中断操作接口
    void (*disable)(unsigned long irq);     //关闭
    unsigned long (*installer)(unsigned long irq,void* arg);
    void (*uninstaller)(unsigned long irq);
    void (*ack)(unsigned long nr);          //应答
} hw_int_controler;


/*  记录处理中断时所必须的信息  */
typedef struct{
    hw_int_controler* controler;//中断的使能、应答、禁止等操作
    char* irq_name;             //中断名
    unsigned long parameter;    //中断处理函数的参数
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs); //中断处理函数
    unsigned long flags;        //标志位
} irq_desc_T;

irq_desc_T interrupt_desc[NR_IRQS] = {0};


bool register_irq(
    unsigned long irq,
    void* arg,
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),
                  unsigned long parameter,
                  hw_int_controler* controler,
                  char* irq_name
);
bool unregister_irq(unsigned long irq);


#endif // !__KERNEL_INTERRUPT_H
