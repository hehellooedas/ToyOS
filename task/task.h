#ifndef __Task_Task_H
#define __Task_Task_H

#include <list.h>
#include <printk.h>


#define STACK_SIZE 32768  //32K


/*  PCB是进程的身份证  */
struct task_struct{
    struct   List       list;    //双向链表
    volatile long       state;   //进程状态
    unsigned long       flags;   //进程标志
    struct mm_struct*   mm;      //内存空间分布结构体
    struct thread_struct* thread; //进程切换时保留的状态信息
    unsigned long addr_limit;    //进程地址空间范围
    long pid;       //进程ID
    long counter;   //时间片
    long signal;    //进程持有的信号
    long priority;  //进程优先级
};



/*    */
struct mm_struct{

    unsigned long start_code,end_code;     //代码段区域
    unsigned long start_data,end_data;     //数据段区域
    unsigned long start_rodata,end_rodata; //只读数据段区域
    unsigned long start_brk,end_brk;       //堆区域
    unsigned long start_stack;             //应用层栈基地址
};



/*  参与进程调度所必须的信息  */
struct thread_struct{
    unsigned long rsp0;   //内核层栈基地址
    unsigned long rip;    //内核层代码指针(进程切换时的rip)
    unsigned long rsp;    //内核层当前栈指针(进程切换时的rsp)

    unsigned long fs;     //FS段寄存器
    unsigned long gs;     //GS段寄存器

    unsigned long cr2;       //cr2控制寄存器
    unsigned long trap_nr;   //产生异常的异常号
    unsigned long error_code;//异常的错误码
};



/*  占用32KB,起始地址按照32KB对齐  */
union task_union{
    struct task_struct task;
    unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
}__attribute__((aligned(8)));



struct tss_struct{
    unsigned int  reserved0;
    unsigned long rsp0;
    unsigned long rsp1;
    unsigned long rsp2;
    unsigned int  reserved1;
    unsigned long ist1;
    unsigned long ist2;
    unsigned long ist3;
    unsigned long ist4;
    unsigned long ist5;
    unsigned long ist6;
    unsigned long ist7;
    unsigned long reserved2;
    unsigned short reserved3;
    unsigned short iomapbaseaddr;
}__attribute__((packed));



/*  函数声明  */

/*  获取当前正在运行的进程的pcb  */
static __attribute__((always_inline))
struct task_struct* get_current()
{
    struct task_struct* current = NULL;
    asm volatile (
        "andq %%rsp,%0  \n\t"
        :"=r"(current)
        :"0"(~32767UL)
    );
    return current;
}
#define running_threaed() get_current()



static __attribute__((always_inline))
void switch_to(struct task_struct* prev,struct task_struct* next)
{
    asm volatile (
        "pushq %%rbp    \n\t"
        "pushq %%rax    \n\t"
        "movq %%rsp,%0  \n\t"
        "movq %2,%%rsp  \n\t"
        "leaq 1f(%%rip),%%rax    \n\t"
        "movq %%rax,%1  \n\t"
        "pushq %3       \n\t"
        "jmp __switch_to \n\t"
        "1:              \n\t"
        "popq %%rbp     \n\t"
        "popq %%rax     \n\t"
        :"=m"(prev->thread->rsp),"=m"(prev->thread->rip)
        :"m"(next->thread->rsp),"m"(next->thread->rip),
         "D"(prev),"S"(next)
        :"memory"
    );
}


static __attribute__((always_inline))
void __switch_to(struct task_struct* prev,struct task_struct* next)
{
    
}


#endif // !__Task_Task_H
