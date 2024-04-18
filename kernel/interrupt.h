#ifndef __KERNEL_INTERRUPT_H
#define __KERNEL_INTERRUPT_H

#include <linkage.h>
#include <printk.h>
#include <ptrace.h>
#include <lib.h>
#include <io.h>
#include <stdbool.h>
#include <flags.h>


void interrupt_init(void);


enum intr_status{
    INTR_OFF,   //关中断状态
    INTR_ON     //开中断状态
};


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



/*  检测当前IF位的状态  */
static __attribute__((always_inline))
enum intr_status get_intr_status(void)
{
    return ((get_rflags().IF) ? INTR_ON:INTR_OFF);
}



static __attribute__((always_inline))
enum intr_status intr_enable()
{
    enum intr_status old_status;
    if(get_intr_status() == INTR_ON){
        old_status = INTR_ON;
    }else{
        old_status = INTR_OFF;
        sti();
    }
    return old_status;
}



static __attribute__((always_inline))
enum intr_status intr_disable()
{
    enum intr_status old_status;
    if(get_intr_status() == INTR_ON){
        old_status = INTR_ON;
        cli();
    }else{
        old_status = INTR_OFF;
    }
    return old_status;
}



static __attribute__((always_inline))
void set_intr_status(enum intr_status status)
{
    if(status == INTR_ON){
        intr_enable();
    }else{
        intr_disable();
    }
}



#define NR_IRQS 24



typedef struct hw_int_type{
    void (*enable)(unsigned long irq);      //使能中断操作接口
    void (*disable)(unsigned long irq);     //关闭
    bool (*installer)(unsigned long irq,void* arg);
    void (*uninstaller)(unsigned long irq);
    void (*ack)(unsigned long nr);          //应答
} hw_int_controller;


/*  记录处理中断时所必须的信息  */
typedef struct{
    hw_int_controller* controller;//中断的使能、应答、禁止等操作
    char* irq_name;             //中断名
    unsigned long parameter;    //中断处理函数的参数
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs); //中断处理函数
    unsigned long flags;        //标志位
} irq_desc_T;


extern irq_desc_T interrupt_desc[NR_IRQS];


bool register_irq(
    unsigned long irq,
    void* arg,
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),
    unsigned long parameter,
    hw_int_controller* controller,
    char* irq_name
);
bool unregister_irq(unsigned long irq);



extern irq_desc_T SMP_IPI_desc[10];

extern void (*SMP_interrupt[10])(void);

bool register_IPI(
    unsigned long irq,
    void* arg,
    void (*handler)(unsigned long nr,unsigned long parameter,struct pt_regs* regs),
    unsigned long parameter,
    hw_int_controller* controller,
    char* irq_name
);


#endif // !__KERNEL_INTERRUPT_H
