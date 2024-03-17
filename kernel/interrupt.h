#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include <linkage.h>
#include <printk.h>
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

/*  记录处理中断时所必须的信息  */
typedef struct{

    char* irq_name;
    unsigned long parameter;

    unsigned long flags;
} irq_desc_T;



#endif // !__KERNEL_INTERRUPT_H
